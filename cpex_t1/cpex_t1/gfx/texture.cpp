/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#include <gfx/texture.hpp>
// #include <zcl/zcl.hpp>
#include <cmath>

// EXTERNAL LIBRARIES //
// ----------------------------
// STB
#include <stb_image.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

Texture::Texture():
    name("<UNNAMED_TEXTURE>"),
    texId(0),
    fmtTarget(GL_TEXTURE_2D),
    fmtInternal(GL_RGBA8) {
        // zcl::logger("TEX")->info("TEXTURE {} CREATED", name);
    }

Texture::Texture(std::string name):
    name(name),
    texId(0),
    fmtTarget(GL_TEXTURE_2D),
    fmtInternal(GL_RGBA8) {
        // zcl::logger("TEX")->info("TEXTURE {} CREATED", name);
    }

Texture::~Texture() {
    // zcl::logger("TEX")->info("TEXTURE {} DESTROYED", name);
    free_resources();
}

Texture& Texture::operator=(Texture &&other) {
    // zcl::logger("TEX")->info("TEXTURE {}<-{} MOVED", name, other.name);

    if (this == &other) {
        return *this;
    }

    std::swap(name, other.name);
    std::swap(texId, other.texId);
    std::swap(fmtTarget, other.fmtTarget);
    std::swap(fmtInternal, other.fmtInternal);
    // other.free_resources(); // will be automatically called @ destructor

    return *this;
}

void Texture::free_resources() {
    // zcl::logger("TEX")->info("TEXTURE {}: free_resources()", name);
    if (texId) {
        glDeleteTextures(1, &texId);
        texId = 0;
    }
}

void Texture::load_from_buffer_2d(const void* pixels, int wid, int hei, GLenum dataFormat, GLenum dataType, int mipmapLevels) {
    // Free previous texture
    if (texId) {
        glDeleteTextures(1, &texId);
    }

    glGenTextures(1, &texId);
    glBindTexture(fmtTarget, texId);

    // `glTexStorage` is only supported for 4.2+
    #ifdef GL_VERSION_4_2
        // Calculate mipmap levels if needed
        mipmapLevels = (mipmapLevels == 0)
                        ? ( 1 + floor(std::log2(std::fmax(wid, hei))) )
                        : mipmapLevels;

        glTexStorage2D(fmtTarget, mipmapLevels, fmtInternal, wid, hei);
        glTexSubImage2D(fmtTarget, 0, 0, 0, wid, hei, dataFormat, dataType, pixels);
    #else
        glTexImage2D(fmtTarget, mipmapLevels, fmtInternal, wid, hei, 0, dataFormat, dataType, pixels);
    #endif

    glGenerateMipmap(fmtTarget);
}

void Texture::set_format(GLenum target, GLint internalFormat) {
    fmtTarget = target;
    fmtInternal = internalFormat;
}

void Texture::bind(GLenum slot) {
    glActiveTexture(slot);
    glBindTexture(fmtTarget, texId);
}

void gfx::texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride) {
    stbi_set_flip_vertically_on_load(true);

    int wid, hei, channels;
    unsigned char* pixels = stbi_load(file.string().c_str(), &wid, &hei, &channels, 0);
    auto isAlphaPresent = channels >= 4;
    
    GLint format = (formatOverride != 0) ? formatOverride : (isAlphaPresent ? GL_RGBA8 : GL_RGB8);
    GLint pixelsFormat = isAlphaPresent ? GL_RGBA : GL_RGB;

    tex.set_format(GL_TEXTURE_2D, format);
    tex.load_from_buffer_2d(pixels, wid, hei, pixelsFormat, GL_UNSIGNED_BYTE);
}