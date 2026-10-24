/**
 * zen::asset_manager - Asset manager
 * ZIK@MMXXVI
 */

#include <memory>
#include <fstream>

// LIBRARIES //
#include <gfx/material.hpp>
#include <zen/asset_manager.hpp>
#include <zmd2/zmd2.hpp>

using namespace zen;

AssetManager::AssetManager():
    AssetManager("") {}

AssetManager::AssetManager(const std::filesystem::path &basePath):
    basePath(basePath) {}

AssetManager::AssetManager(AssetManager &&other):
    loadedTextures(std::move(other.loadedTextures)),
    loadedShaders(std::move(other.loadedShaders)),
    loadedMaterials(std::move(other.loadedMaterials)),
    loadedModelsZmd2(std::move(other.loadedModelsZmd2)) {}

AssetManager::~AssetManager() {
    free_resources();
}

AssetManager& AssetManager::operator=(AssetManager &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }

    loadedTextures = std::move(other.loadedTextures);
    loadedShaders = std::move(other.loadedShaders);
    loadedMaterials = std::move(other.loadedMaterials);
    loadedModelsZmd2 = std::move(other.loadedModelsZmd2);

    return *this;
}

void AssetManager::set_base_path(const std::filesystem::path &basePath) {
    this->basePath = basePath;
}

void AssetManager::free_resources() {
    for (auto&& res: loadedModelsZmd2) {
        res.second->free_resources();
    }
    for (auto&& res: loadedTextures) {
        res.second->free_resources();
    }
    for (auto&& res: loadedShaders) {
        res.second->free_resources();
    }
    for (auto&& res: loadedMaterials) {
        res.second->free_resources();
    }

    loadedTextures.clear();
    loadedShaders.clear();
    loadedMaterials.clear();
    loadedModelsZmd2.clear();
}

std::shared_ptr<gfx::Texture> AssetManager::get_texture(const std::string &relPath) {
    if (auto cache = loadedTextures.find(relPath); cache != loadedTextures.end()) {
        // Cache hit
        return cache->second;
    }

    return nullptr;
}

std::shared_ptr<gfx::Texture> AssetManager::load_texture(const std::string &relPath, bool forceReload) {
    if (auto tex = get_texture(relPath); !forceReload && tex) {
        return tex;
    }

    auto texPath = basePath / relPath;
    auto tex = gfx::Texture(relPath, GL_TEXTURE_2D);

    if (std::filesystem::is_directory(texPath) || !std::filesystem::exists(texPath)) {
        zcl::logger("ASSET")->error("TEXTURE DOES NOT EXIST! (path: `{}`)", texPath.string());
        throw std::runtime_error(fmt::format("TEXTURE DOES NOT EXIST! (path: `{}`)", texPath.string()));
    }
    // zcl::logger("ASSET")->info("LOAD TEXTURE FROM `{}`", texPath.string());

    gfx::texhelper::texture_load_from_file_2d(tex, texPath);

    auto texPtr = std::make_shared<gfx::Texture>(std::move(tex));

    loadedTextures[relPath] = texPtr;
    return texPtr;
}

void AssetManager::add_texture(const std::string &relPath, const std::shared_ptr<gfx::Texture> &data) {
    // zcl::logger("ASSET")->info("ADD TEXTURE `{}`", relPath);
    loadedTextures[relPath] = data;
}

std::shared_ptr<gfx::Shader> AssetManager::get_shader(const std::string &relPath) {
    if (auto cache = loadedShaders.find(relPath); cache != loadedShaders.end()) {
        // Cache hit
        return cache->second;
    }

    return nullptr;
}

std::shared_ptr<gfx::Shader> AssetManager::load_shader(const std::string &relPath, bool forceReload) {
    if (auto shd = get_shader(relPath); !forceReload && shd) {
        return shd;
    }

    auto shd = gfx::Shader(relPath);
    auto shdPtr = std::make_shared<gfx::Shader>(std::move(shd));

    try {
        if (auto vertPath = basePath / (relPath + ".vert"); std::filesystem::exists(vertPath)) {
            shdPtr->load_shader_from(vertPath, GL_VERTEX_SHADER);
        }
        if (auto fragPath = basePath / (relPath + ".frag"); std::filesystem::exists(fragPath)) {
            shdPtr->load_shader_from(fragPath, GL_FRAGMENT_SHADER);
        }
        shdPtr->link_program();
    }
    catch (std::runtime_error err) {
        zcl::logger("ASSET")->error("FAILED TO PREPARE SHADER!\n{}", std::string(err.what()));
        throw std::runtime_error(fmt::format("FAILED TO PREPARE SHADER!\n{}", std::string(err.what())));
    }

    loadedShaders[relPath] = shdPtr;
    return shdPtr;
}

void AssetManager::add_shader(const std::string &relPath, const std::shared_ptr<gfx::Shader> &data) {
    loadedShaders[relPath] = data;
}

std::shared_ptr<gfx::Material> AssetManager::get_material(const std::string &relPath) {
    if (auto cache = loadedMaterials.find(relPath); cache != loadedMaterials.end()) {
        // Cache hit
        return cache->second;
    }

    return nullptr;
}

void AssetManager::add_material(const std::string &relPath, const std::shared_ptr<gfx::Material> &data) {
    loadedMaterials[relPath] = data;
}

std::shared_ptr<gfx::zmd2::Model> AssetManager::get_model_zmd2(const std::string &relPath) { 
    if (auto cache = loadedModelsZmd2.find(relPath); cache != loadedModelsZmd2.end()) {
        // Cache hit
        return cache->second;
    }

    return nullptr;
}

std::shared_ptr<gfx::zmd2::Model> AssetManager::load_model_zmd2(const std::string relPath, bool forceReload, const std::string &baseMaterialId, const std::string &fallbackMaterialId, const std::string &fallbackTextureId, const std::string &fallbackShaderId) { 
    if (auto mdl = get_model_zmd2(relPath); !forceReload && mdl) {
        return mdl;
    }

    auto mdlPtr = std::shared_ptr<gfx::zmd2::Model>(nullptr);

    try {
        if (auto file = std::ifstream(basePath/ relPath, std::ios_base::binary); file) {
            auto matBase = get_material(baseMaterialId);
            auto matFallback = get_material(fallbackMaterialId);
            auto texFallback = load_texture(fallbackTextureId);
            auto shdFallback = load_shader(fallbackShaderId);

            assert(matFallback != nullptr && texFallback != nullptr && "Fallback assets does not exist!");

            auto mdl = zmd2::load_model_from(relPath, file);
            auto mdlConverted = gfx::zmd2::Model(&mdl);
            auto extraBytes = mdlConverted.get_extra_bytes();

            // Load embedded assets
            // (texture)
            for (auto&& tex: mdlConverted.all_embedded_textures()) {
                auto texConverted = tex.to_texture(extraBytes);
                auto texConvertedPtr = std::make_shared<gfx::Texture>(std::move(texConverted));

                add_texture(tex.name, texConvertedPtr);
                mdlConverted.link_texture(tex.name, texConvertedPtr);
            }
            // (material)
            for (auto&& mat: mdlConverted.all_embedded_materials()) {
                // auto matConverted = mat.to_material();
                auto matConvertedPtr = std::make_shared<gfx::Material>(mat.to_material());
                auto shaderId = mat.shader;
                auto baseTexId = mat.baseTexture;
                auto assetShd = load_shader(shaderId);
                std::shared_ptr<gfx::Texture> assetBaseTex;

                try {
                    assetBaseTex = load_texture(baseTexId);
                }
                catch (std::exception e) {
                    assetBaseTex = texFallback;
                }

                if (!assetShd) {
                    assetShd = shdFallback;
                }
                gfx::material_set_inherited(matBase, matConvertedPtr);
                matConvertedPtr->set_shader(assetShd);

                // (replace / link textures for samplers)
                for (auto&& uniform: matConvertedPtr->get_uniforms(false)) {
                    if (auto samplerUniform = std::dynamic_pointer_cast<gfx::UniformSampler>(uniform); samplerUniform && samplerUniform->get_value()) {
                        auto texId = samplerUniform->get_value()->get_id();
                        auto assetTex = get_texture(texId);

                        if (!assetTex) {
                            zcl::logger("ASSET")->error("{} (MATERIAL `{}`): FAILED TO LOAD SAMPLER `{}` DEFAULT TEXTURE (`{}`), USING FALLBACK...", mdl.id, mat.name, uniform->get_name(), texId);
                            assetTex = texFallback;
                        }

                        samplerUniform->set_value(assetTex);
                    }
                }

                // Add base texture uniform if there isn't any
                if (auto uniformBaseTex = matConvertedPtr->find_uniform_by_name<gfx::UniformSampler>(ZMD2_UNIFORM_NAME_BASE_TEXTURE, false); !uniformBaseTex) {
                    matConvertedPtr->add_uniforms(
                        gfx::UniformSampler(ZMD2_UNIFORM_NAME_BASE_TEXTURE, assetBaseTex)
                    );
                }

                add_material(mat.name, matConvertedPtr);
            }
            for (auto i = 0; i < mdlConverted.all_material_ids().size(); i++) {
                auto matId = mdlConverted.all_material_ids()[i];
                auto assetMat = get_material(matId);

                mdlConverted.link_material(i, assetMat);
            }

            mdlConverted.update_refs_and_link_assets(matFallback);
            mdlPtr = std::make_shared<gfx::zmd2::Model>(std::move(mdlConverted));
            loadedModelsZmd2[relPath] = mdlPtr;
        }
        else {
            throw std::runtime_error(fmt::format("Model `{}` does not exist!", relPath));
        }        
    }
    catch (std::runtime_error err) {
        zcl::logger("ASSET")->error("FAILED TO LOAD MODEL!\n{}", std::string(err.what()));
        throw std::runtime_error(fmt::format("FAILED TO LOAD MODEL!\n{}", std::string(err.what())));
    }

    return mdlPtr;
}

void AssetManager::add_model_zmd2(const std::string &relPath, const std::shared_ptr<gfx::zmd2::Model> &data) { 
    loadedModelsZmd2[relPath] = data;
}

const std::unordered_map<std::string, std::shared_ptr<gfx::Texture>> AssetManager::all_loaded_textures() const {
    return loadedTextures;
}