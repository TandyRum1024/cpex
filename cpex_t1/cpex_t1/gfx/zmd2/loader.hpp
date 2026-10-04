/**
 * gfx::zmd2::loader - `.zmd2` model loading & wrapper for this projects graphics library.
 * ZIK@MMXXVI
 */

#ifndef __GFX_ZMD2_LOADER_GUARD
#define __GFX_ZMD2_LOADER_GUARD

#include <filesystem>

#include <zmd2/types.hpp>
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/vb.hpp>

// LIBRARIES //
#include <zen/asset_manager.hpp>

namespace gfx {
    namespace zmd2 {
        /** Run this on init */
        void init(zen::AssetManager &manager, const std::filesystem::path &assetPath);
    }
}

#endif