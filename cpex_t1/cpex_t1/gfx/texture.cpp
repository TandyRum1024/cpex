/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#include <cmath>

#include <gfx/texture.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

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
    texSlot(GL_TEXTURE0),
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
    texSlot(std::exchange(other.texSlot, 0)),
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
    std::swap(texSlot, other.texSlot);
    std::swap(fmtInternal, other.fmtInternal);
    // other.free_resources(); // will be automatically called @ destructor

    return *this;
}

GLuint Texture::get_texture_id() const {
    return texId;
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
    texSlot = slot;
    glActiveTexture(texSlot);
    glBindTexture(texTarget, texId);
}

void Texture::unbind() {
    glActiveTexture(texSlot);
    glBindTexture(texTarget, 0);
}

GLenum Texture::get_bound_slot() const {
    return texSlot;
}

GLenum Texture::get_target() const {
    return texTarget;
}

TextureManager::TextureManager():
    currentUnitIdx(0) {};

TextureManager::~TextureManager() {
    clear();
}

unsigned int TextureManager::bind_texture(const std::weak_ptr<Texture> &tex) {
    auto texture = tex.lock();

    if (!texture) {
        return 0;
    }

    // Check for already bound texture
    auto texId = texture->get_texture_id();

    if (auto slot = slotsAllocatedByTexId.find(texId); slot != slotsAllocatedByTexId.end()) {
        return slot->second;
    }

    bool mayRecycle = !slotsRecycled.empty();
    unsigned int newSlot;

    // On overflow, release the earliest texture
    if (currentUnitIdx >= TEXTURES_MAX && !mayRecycle) {
        unbind_texture(0);
        mayRecycle = !slotsRecycled.empty();
    }
    
    // Check for available slots
    if (mayRecycle) {
        newSlot = slotsRecycled.back();
        slotsRecycled.pop_back();
    }
    else {
        newSlot = currentUnitIdx;
        currentUnitIdx++;
    }

    // zcl::logger("TEX")->info("newSlot {}", newSlot);
    slotsAllocated[newSlot] = tex;
    slotsAllocatedByTexId[texId] = newSlot;
    texture->bind(GL_TEXTURE0 + newSlot);
    return newSlot;
}

void TextureManager::unbind_texture(unsigned int slot) {
    if (!slotsAllocated.contains(slot)) {
        return;
    }
    
    if (auto texture = slotsAllocated.at(slot).lock()) {
        texture->unbind();
        slotsAllocatedByTexId.erase(texture->get_texture_id());
    }
    slotsAllocated.erase(slot);
    slotsRecycled.push_back(slot);
}

void TextureManager::clear() {
    for (auto&& entry: slotsAllocated) {
        if (auto texture = entry.second.lock()) {
            texture->unbind();
        }
    }

    currentUnitIdx = 0;
    slotsAllocated.clear();
    slotsAllocatedByTexId.clear();
    slotsRecycled.clear();
}

unsigned int TextureManager::get_allocated_num() const {
    return slotsAllocated.size();
}

void texhelper::texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride) {
    stbi_set_flip_vertically_on_load(true);

    int wid, hei, channels;
    unsigned char* pixels = stbi_load(file.string().c_str(), &wid, &hei, &channels, 0);
    auto isAlphaPresent = channels >= 4;
    
    GLint format = (formatOverride != 0) ? formatOverride : (isAlphaPresent ? GL_RGBA8 : GL_RGB8);
    GLint pixelsFormat = isAlphaPresent ? GL_RGBA : GL_RGB;

    tex.set_format(format);
    tex.load_from_buffer_2d(pixels, wid, hei, pixelsFormat, GL_UNSIGNED_BYTE);
}