/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#include <cmath>
#include <stdint.h>

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
    texUnit(GL_TEXTURE0),
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
    texUnit(std::exchange(other.texUnit, 0)),
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
    std::swap(texUnit, other.texUnit);
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

void Texture::bind(GLenum unit) {
    texUnit = unit;
    glActiveTexture(texUnit);
    glBindTexture(texTarget, texId);
}

void Texture::unbind() {
    glActiveTexture(texUnit);
    glBindTexture(texTarget, 0);
}

GLenum Texture::get_bound_unit() const {
    return texUnit;
}

GLenum Texture::get_target() const {
    return texTarget;
}

int TextureManager::SLOTS_MAX = 0;

TextureManager::TextureManager():
    currentSlotIdx(0) {};

TextureManager::~TextureManager() {
    clear();
}

void TextureManager::init() {
    if (!TextureManager::SLOTS_MAX) {
        int maxUnits = 0;

        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
        TextureManager::SLOTS_MAX = maxUnits;
    }
}

unsigned int TextureManager::bind_texture(const std::weak_ptr<Texture> &tex, const GLuint keySalt) {
    auto texture = tex.lock();

    if (!texture) {
        return 0;
    }

    // Check for already bound texture
    GLuint texId = texture->get_texture_id();

    // TODO: this is only possible on 64bit machines as texId is 32bit -> making key only possible with 64bit values
    int64_t key = (texId << sizeof(GLuint)) | keySalt;

    if (auto slot = slotsAllocatedTbl.find(key); slot != slotsAllocatedTbl.end()) {
        return slot->second;
    }

    bool mayRecycle = !slotsRecycled.empty();
    unsigned int newSlot;

    // On overflow, release the earliest texture
    if (currentSlotIdx >= SLOTS_MAX && !mayRecycle) {
        unbind_texture(0);
        mayRecycle = !slotsRecycled.empty();
    }
    
    // Check for available slots
    if (mayRecycle) {
        newSlot = slotsRecycled.back();
        slotsRecycled.pop_back();
    }
    else {
        newSlot = currentSlotIdx;
        currentSlotIdx++;
    }

    // zcl::logger("TEX")->info("newSlot {}", newSlot);
    slotsAllocated[newSlot] = TextureAndKey { .texture = tex, .key = key };
    slotsAllocatedTbl[key] = newSlot;
    texture->bind(GL_TEXTURE0 + newSlot);
    return newSlot;
}

void TextureManager::unbind_texture(unsigned int slot) {
    if (!slotsAllocated.contains(slot)) {
        return;
    }
    
    if (auto texKey = slotsAllocated.at(slot); auto texture = texKey.texture.lock()) {
        texture->unbind();
        slotsAllocatedTbl.erase(texKey.key);
    }
    slotsAllocated.erase(slot);
    slotsRecycled.push_back(slot);
}

void TextureManager::clear() {
    for (auto&& entry: slotsAllocated) {
        if (auto texture = entry.second.texture.lock()) {
            texture->unbind();
        }
    }

    currentSlotIdx = 0;
    slotsAllocated.clear();
    slotsAllocatedTbl.clear();
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