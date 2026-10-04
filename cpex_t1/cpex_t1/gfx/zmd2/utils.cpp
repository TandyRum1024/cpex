/**
 * gfx::zmd2::utils - `.zmd2` model utils.
 * ZIK@MMXXVI
 */

#include <gfx/zmd2/utils.hpp>

// Table to convert model type to vertex format.
const std::unordered_map<zmd2::MODEL_TYPE, std::shared_ptr<gfx::VertFormat>> ZMD2_FORMAT_BY_MODEL_TBL = {
        { ::zmd2::MODEL_TYPE_MESH, gfx::zmd2::ZMD2_VERT_FORMAT_MESH },
        { ::zmd2::MODEL_TYPE_MESH_MORPH_SKINNED, gfx::zmd2::ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
        { ::zmd2::MODEL_TYPE_WIRE_MORPH_SKINNED, gfx::zmd2::ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
    };

// Table to convert primitive type to OpenGL primitive mode.
const std::unordered_map<zmd2::PRIM_TYPE, GLenum> ZMD2_GL_PRIM_BY_PRIM_TBL = {
        { ::zmd2::PRIM_TYPE_TRIANGLE_LIST, GL_TRIANGLES },
        { ::zmd2::PRIM_TYPE_LINE_LIST, GL_LINES },
    };

// Table to convert texture format to OpenGL BASE INTERNAL texture format.
const std::unordered_map<gfx::zmd2::ZMD2_TEX_FORMAT, GLenum> ZMD2_TEX_FORMAT_TO_GL_TEX_INTERNAL_FORMAT_TBL = {
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA8_UNORM, GL_RGBA8 },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R8_UNORM, GL_R8 },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RG8_UNORM, GL_RG8 },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA4_UNORM, GL_RGBA4 },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA16_FLOAT, GL_RGBA16F },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R16_FLOAT, GL_R16 },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA32_FLOAT, GL_RGBA32F },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R32_FLOAT, GL_R32F },
    };

// Table to convert texture format to OpenGL PIXEL DATA texture format.
const std::unordered_map<gfx::zmd2::ZMD2_TEX_FORMAT, GLenum> ZMD2_TEX_FORMAT_TO_GL_TEX_FORMAT_TBL = {
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA8_UNORM, GL_RGBA },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R8_UNORM, GL_RED },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RG8_UNORM, GL_RG },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA4_UNORM, GL_RGBA },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA16_FLOAT, GL_RGBA },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R16_FLOAT, GL_RED },
        { gfx::zmd2::ZMD2_TEX_FORMAT_RGBA32_FLOAT, GL_RGBA },
        { gfx::zmd2::ZMD2_TEX_FORMAT_R32_FLOAT, GL_RED },
    };

GLenum gfx::zmd2::find_gl_prim_by_prim_type(const ::zmd2::PRIM_TYPE &type) {
    if (auto found = ZMD2_GL_PRIM_BY_PRIM_TBL.find(type); found != ZMD2_GL_PRIM_BY_PRIM_TBL.end()) {
        return found->second;
    }
    return 0;
}

std::shared_ptr<gfx::VertFormat> gfx::zmd2::find_vert_format_by_model_type(const ::zmd2::MODEL_TYPE &type) {
    if (auto found = ZMD2_FORMAT_BY_MODEL_TBL.find(type); found != ZMD2_FORMAT_BY_MODEL_TBL.end()) {
        return found->second;
    }
    return 0;
}

GLenum gfx::zmd2::find_gl_tex_base_format_by_tex_format(const ZMD2_TEX_FORMAT &type) {
    if (auto found = ZMD2_TEX_FORMAT_TO_GL_TEX_INTERNAL_FORMAT_TBL.find(type); found != ZMD2_TEX_FORMAT_TO_GL_TEX_INTERNAL_FORMAT_TBL.end()) {
        return found->second;
    }
    return GL_RGBA8;
}

GLenum gfx::zmd2::find_gl_tex_data_format_by_tex_format(const ZMD2_TEX_FORMAT &type) {
    if (auto found = ZMD2_TEX_FORMAT_TO_GL_TEX_FORMAT_TBL.find(type); found != ZMD2_TEX_FORMAT_TO_GL_TEX_FORMAT_TBL.end()) {
        return found->second;
    }
    return GL_RGBA8;
}