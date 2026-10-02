/**
 * zen::asset_manager - Asset manager
 * ZIK@MMXXVI
 */

#ifndef __ZEN_ASSETMGR_GUARD
#define __ZEN_ASSETMGR_GUARD

#include <string>
#include <filesystem>
#include <unordered_map>

// LIBRARIES //
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/shader.hpp>
#include <zmd2_model.hpp>

namespace zen {
    class AssetManager {
        std::filesystem::path basePath;
        
        // Cache of already loaded assets

        std::unordered_map<std::string, std::shared_ptr<gfx::Texture>> loadedTextures;
        std::unordered_map<std::string, std::shared_ptr<gfx::Shader>> loadedShaders;
        std::unordered_map<std::string, std::shared_ptr<gfx::Material>> loadedMaterials;
        std::unordered_map<std::string, std::shared_ptr<mdl::zmd2::Model>> loadedModelsZmd2;

    public:
        AssetManager();
        AssetManager(const std::filesystem::path &basePath);
        // We do NOT want to accidentally copy the asset manager along its loaded assets for a very obvious reasons
        // So we intentionally delete the copy ops and recommend the move ops

        AssetManager(AssetManager &&other);
        AssetManager& operator=(AssetManager &&other);

        AssetManager(const AssetManager &other) = delete;
        AssetManager& operator=(const AssetManager &other) = delete;

        void set_base_path(const std::filesystem::path &basePath);

        // Now for bunch of asset loading / fetching functions

        std::shared_ptr<gfx::Texture> get_texture(const std::string &relPath);
        std::shared_ptr<gfx::Texture> load_texture(const std::string &relPath, bool forceReload = false);
        void add_texture(const std::string &relPath, const std::shared_ptr<gfx::Texture> &data);

        std::shared_ptr<gfx::Shader> get_shader(const std::string &relPath);
        std::shared_ptr<gfx::Shader> load_shader(const std::string &relPath, bool forceReload = false);
        void add_shader(const std::string &relPath, const std::shared_ptr<gfx::Shader> &data);

        std::shared_ptr<gfx::Material> get_material(const std::string &relPath);
        //std::shared_ptr<gfx::Material> load_material(const std::string relPath, bool forceReload = false);
        void add_material(const std::string &relPath, const std::shared_ptr<gfx::Material> &data);

        std::shared_ptr<mdl::zmd2::Model> get_model_zmd2(const std::string &relPath);
        std::shared_ptr<mdl::zmd2::Model> load_model_zmd2(const std::string relPath, bool forceReload = false);
        void add_model_zmd2(const std::string &relPath, const std::shared_ptr<mdl::zmd2::Model> &data);
    };
}

#endif