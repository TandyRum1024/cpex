/**
 * gfx::zmd2::utils - `.zmd2` model utils.
 * ZIK@MMXXVI
 */

#ifndef __GFX_ZMD2_UTILS_GUARD
#define __GFX_ZMD2_UTILS_GUARD

#include <map>

#include <gfx/zmd2/model.hpp>

// LIBRARIES //
#include <gfx/vb.hpp>
#include <zmd2/types.hpp>

namespace gfx {
    namespace zmd2 {
        /** Returns OpenGL primitive mode from given primitive type. `0` if not found. */
        GLenum find_gl_prim_by_prim_type(const ::zmd2::PRIM_TYPE &type);

        /** Returns vertex format from given model type. `nullptr` if not found. */
        std::shared_ptr<gfx::VertFormat> find_vert_format_by_model_type(const ::zmd2::MODEL_TYPE &type);

        /** Returns OpenGL texture format from given texture format. `GL_RGBA8` if not found. */
        GLenum find_gl_tex_base_format_by_tex_format(const ZMD2_TEX_FORMAT &type);

        /** Returns OpenGL INTERNAL texture format from given texture format. `GL_RGBA8` if not found. */
        GLenum find_gl_tex_data_format_by_tex_format(const ZMD2_TEX_FORMAT &type);
    }
}
#endif