/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#include <cmath>

#include <gfx/texture.hpp>

// LIBRARIES //
// #include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// STB
#include <stb_image.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

Texture::Texture():
    Texture("<UNNAMED_TEXTURE>", GL_TEXTURE_2D) {
    // zcl::logger("TEX")->info("TEXTURE {} CREATED", name);
}

Texture::Texture(std::string name):
    Texture(name, GL_TEXTURE_2D) {
    // zcl::logger("TEX")->info("TEXTURE {} CREATED", name);
}

Texture::Texture(std::string name, GLenum texTarget):
    name(name),
    texId(0),
    texTarget(texTarget),
    fmtInternal(GL_RGBA8)
    {
    // zcl::logger("TEX")->info("TEXTURE {} CREATED", name);
}

Texture::~Texture() {
    // zcl::logger("TEX")->info("TEXTURE {} DESTROYED", name);
    free_resources();
}

Texture::Texture(Texture &&other):
    name(std::move(other.name)),
    texId(std::exchange(other.texId, 0)),
    texTarget(std::exchange(other.texTarget, 0)),
    fmtInternal(std::exchange(other.fmtInternal, 0)) {}

Texture& Texture::operator=(Texture &&other) {
    // zcl::logger("TEX")->info("TEXTURE {}<-{} MOVED", name, other.name);

    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }

    std::swap(name, other.name);
    std::swap(texId, other.texId);
    std::swap(texTarget, other.texTarget);
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
    glBindTexture(texTarget, texId);

    // `glTexStorage` is only supported for 4.2+
    #ifdef GL_VERSION_4_2
        // Calculate mipmap levels if needed
        mipmapLevels = (mipmapLevels == 0)
                        ? ( 1 + floor(std::log2(std::fmax(wid, hei))) )
                        : mipmapLevels;

        glTexStorage2D(texTarget, mipmapLevels, fmtInternal, wid, hei);
        glTexSubImage2D(texTarget, 0, 0, 0, wid, hei, dataFormat, dataType, pixels);
    #else
        glTexImage2D(texTarget, mipmapLevels, fmtInternal, wid, hei, 0, dataFormat, dataType, pixels);
    #endif

    glGenerateMipmap(texTarget);
}

void Texture::set_format(GLint internalFormat) {
    fmtInternal = internalFormat;
}

void Texture::bind(GLenum slot) {
    glActiveTexture(slot);
    glBindTexture(texTarget, texId);
}

void Texture::set_texture_param(GLint texFilterMode, GLint texWrapMode) {
    glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, texFilterMode);
    glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, texFilterMode);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, texWrapMode);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, texWrapMode);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_R, texWrapMode);
}

void gfx::texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride) {
    stbi_set_flip_vertically_on_load(true);

    int wid, hei, channels;
    unsigned char* pixels = stbi_load(file.string().c_str(), &wid, &hei, &channels, 0);
    auto isAlphaPresent = channels >= 4;
    
    GLint format = (formatOverride != 0) ? formatOverride : (isAlphaPresent ? GL_RGBA8 : GL_RGB8);
    GLint pixelsFormat = isAlphaPresent ? GL_RGBA : GL_RGB;

    tex.set_format(format);
    tex.load_from_buffer_2d(pixels, wid, hei, pixelsFormat, GL_UNSIGNED_BYTE);
}