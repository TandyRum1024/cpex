/**
 * zen::asset_manager - Asset manager
 * ZIK@MMXXVI
 */

#include <memory>

#include <zen/asset_manager.hpp>

// LIBRARIES //
#include <gfx/texture.hpp>

using namespace zen;

AssetManager::AssetManager():
    AssetManager("") {}

AssetManager::AssetManager(const std::filesystem::path &basePath):
    basePath(basePath) {}

AssetManager::AssetManager(AssetManager &&other):
    loadedTextures(std::move(other.loadedTextures)),
    loadedShaders(std::move(other.loadedShaders)),
    loadedMaterials(std::move(other.loadedMaterials)) {}

AssetManager& AssetManager::operator=(AssetManager &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }

    loadedTextures = std::move(other.loadedTextures);
    loadedShaders = std::move(other.loadedShaders);
    loadedMaterials = std::move(other.loadedMaterials);

    return *this;
}

void AssetManager::set_base_path(const std::filesystem::path &basePath) {
    this->basePath = basePath;
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

    auto tex = gfx::Texture(relPath, GL_TEXTURE_2D);

    gfx::texhelper::texture_load_from_file_2d(tex, basePath / relPath);

    auto texPtr = std::make_shared<gfx::Texture>(std::move(tex));

    loadedTextures[relPath] = texPtr;
    return texPtr;
}

void AssetManager::add_texture(const std::string &relPath, const std::shared_ptr<gfx::Texture> &data) {
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