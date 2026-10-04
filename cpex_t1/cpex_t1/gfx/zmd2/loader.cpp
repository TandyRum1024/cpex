/**
 * gfx::zmd2::loader - `.zmd2` model loading & wrapper for this projects graphics library.
 * ZIK@MMXXVI
 */

#include <gfx/zmd2/loader.hpp>
#include <gfx/zmd2/config.hpp>
#include <gfx/uniform.hpp>

using namespace gfx::zmd2;

void gfx::zmd2::init(zen::AssetManager &manager, const std::filesystem::path &assetPath) {
    auto shd = manager.load_shader(ZMD2_ASSET_BASE_MATERIAL_SHADER);
    auto mat = std::make_shared<gfx::Material>(ZMD2_ASSET_BASE_MATERIAL);
    auto tex = std::make_shared<gfx::Texture>(ZMD2_ASSET_FALLBACK_TEXTURE);

    gfx::texhelper::texture_load_from_file_2d(*tex, assetPath / ZMD2_ASSET_FALLBACK_TEXTURE_PATH);

    mat->set_shader(shd);
    mat->add_uniforms(
        gfx::UniformMat4("uMatModel", glm::mat4(1.0f)),
        gfx::UniformMat4("uMatView", glm::mat4(1.0f)),
        gfx::UniformMat4("uMatProjection", glm::mat4(1.0f)),
        gfx::UniformSampler(ZMD2_UNIFORM_NAME_BASE_TEXTURE, tex, GL_NEAREST, GL_REPEAT)
    );
    manager.add_material(ZMD2_ASSET_BASE_MATERIAL, mat);
    manager.add_texture(ZMD2_ASSET_FALLBACK_TEXTURE, tex);
}
