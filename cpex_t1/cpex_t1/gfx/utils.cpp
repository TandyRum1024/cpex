/**
 * gfx::utils - OpenGL helpers (tables etc)
 * ZIK@MMXXVI
 */

#include <unordered_map>

#include <gfx/utils.hpp>

using namespace gfx;

// (TODO: make this into a table?)

std::string gfx::get_name_from_gl_tex_format(const GLint &format) {
    switch (format) {
        case GL_DEPTH_COMPONENT:
            return "GL_DEPTH_COMPONENT";
        case GL_DEPTH_COMPONENT16:
            return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT24:
            return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT32:
            return "GL_DEPTH_COMPONENT32";
        case GL_DEPTH_COMPONENT32F:
            return "GL_DEPTH_COMPONENT32F";
        case GL_DEPTH_STENCIL:
            return "GL_DEPTH_STENCIL";
        
        case GL_RED:
            return "GL_RED";
        case GL_RG:
            return ":";
        case GL_RGB:
            return "GL_RGB";
        case GL_RGBA:
            return "GL_RGBA";
        case GL_R8:
            return ":";
        case GL_R8_SNORM:
            return "GL_R8_SNORM";
        case GL_R16:
            return "GL_R16";
        case GL_R16_SNORM:
            return "GL_R16_SNORM";
        case GL_RG8:
            return "GL_RG8";
        case GL_RG8_SNORM:
            return "GL_RG8_SNORM";
        case GL_RG16:
            return "GL_RG16";
        case GL_RG16_SNORM:
            return "GL_RG16_SNORM";
        case GL_R3_G3_B2:
            return "GL_R3_G3_B2";
        case GL_RGB4:
            return "GL_RGB4";
        case GL_RGB5:
            return "GL_RGB5";
        case GL_RGB8:
            return "GL_RGB8";
        case GL_RGB8_SNORM:
            return "GL_RGB8_SNORM";
        case GL_RGB10:
            return "GL_RGB10";
        case GL_RGB12:
            return "GL_RGB12";
        case GL_RGB16_SNORM:
            return "GL_RGB16_SNORM";
        case GL_RGBA2:
            return "GL_RGBA2";
        case GL_RGBA4:
            return "GL_RGBA4";
        case GL_RGB5_A1:
            return "GL_RGB5_A1";
        case GL_RGBA8:
            return "GL_RGBA8";
        case GL_RGBA8_SNORM:
            return "GL_RGBA8_SNORM";
        case GL_RGB10_A2:
            return "GL_RGB10_A2";
        case GL_RGB10_A2UI:
            return "GL_RGB10_A2UI";
        case GL_RGBA12:
            return "GL_RGBA12";
        case GL_RGBA16:
            return "GL_RGBA16";
        case GL_SRGB8:
            return "GL_SRGB8";
        case GL_SRGB8_ALPHA8:
            return "GL_SRGB8_ALPHA8";
        case GL_R16F:
            return "GL_R16F";
        case GL_RG16F:
            return "GL_RG16F";
        case GL_RGB16F:
            return "GL_RGB16F";
        case GL_RGBA16F:
            return "GL_RGBA16F";
        case GL_R32F:
            return "GL_R32F";
        case GL_RG32F:
            return "GL_RG32F";
        case GL_RGB32F:
            return "GL_RGB32F";
        case GL_RGBA32F:
            return "GL_RGBA32F";
        case GL_R11F_G11F_B10F:
            return "GL_R11F_G11F_B10F";
        case GL_RGB9_E5:
            return "GL_RGB9_E5";
        case GL_R8I:
            return "GL_R8I";
        case GL_R8UI:
            return "GL_R8UI";
        case GL_R16I:
            return "GL_R16I";
        case GL_R16UI:
            return "GL_R16UI";
        case GL_R32I:
            return "GL_R32I";
        case GL_R32UI:
            return "GL_R32UI";
        case GL_RG8I:
            return "GL_RG8I";
        case GL_RG8UI:
            return "GL_RG8UI";
        case GL_RG16I:
            return "GL_RG16I";
        case GL_RG16UI:
            return "GL_RG16UI";
        case GL_RG32I:
            return "GL_RG32I";
        case GL_RG32UI:
            return "GL_RG32UI";
        case GL_RGB8I:
            return "GL_RGB8I";
        case GL_RGB8UI:
            return "GL_RGB8UI";
        case GL_RGB16I:
            return "GL_RGB16I";
        case GL_RGB16UI:
            return "GL_RGB16UI";
        case GL_RGB32I:
            return "GL_RGB32I";
        case GL_RGB32UI:
            return "GL_RGB32UI";
        case GL_RGBA8I:
            return "GL_RGBA8I";
        case GL_RGBA8UI:
            return "GL_RGBA8UI";
        case GL_RGBA16I:
            return "GL_RGBA16I";
        case GL_RGBA16UI:
            return "GL_RGBA16UI";
        case GL_RGBA32I:
            return "GL_RGBA32I";
        case GL_RGBA32UI:
            return "GL_RGBA32UI";
    }

    return "";
}