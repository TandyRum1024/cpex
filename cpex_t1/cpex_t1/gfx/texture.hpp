/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_TEX_GUARD
#define __CPEX_GFX_TEX_GUARD

#include <stdint.h>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <filesystem>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Texture abstraction. */
    class Texture {
        std::string name;
        
        // OpenGL object refs
        GLuint texId;
        GLenum texTarget;
        GLenum texUnit;
        
        // Format
        GLint fmtInternal;
        
    public:
        Texture();
        Texture(std::string name);
        Texture(std::string name, GLenum texTarget);
        ~Texture();
        
        // Disable default copy ops, since `Texture` is move only (tied to OpenGL objects that are hard to copy)!

        Texture(const Texture &other) = delete;
        Texture& operator=(const Texture &other) = delete;

        // Only implement move ops for now

        Texture(Texture &&other);
        Texture& operator=(Texture &&other);

        /** Returns OpenGL texture ID. */
        GLuint get_texture_id() const;

        /** Frees OpenGL resources. */
        void free_resources();
        
        /** Load from given pixel buffer. */
        void load_from_buffer_2d(const void* pixels, int wid, int hei, GLenum dataFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE, int mipmapLevels = 0);
        /** Set OpenGL texture target and format. */
        void set_format(GLint internalFormat = GL_RGBA8);
        
        /** Bind this texture to given unit. */
        void bind(GLenum unit);
        /** Unbind this texture to given unit. */
        void unbind();

        /** Returns the unit that this texture was bound to. */
        GLenum get_bound_unit() const;
        /** Returns the target this texture was assigned to. */
        GLenum get_target() const;
    };

    /** Texture slot/units manager. Implements dead simple (recycled) index management. */
    class TextureManager {
        /** Texture - key pair */
        struct TextureAndKey {
            std::weak_ptr<gfx::Texture> texture;
            int64_t key;
        };

        // For clarification, texture units = [ GL_TEXTURE0 .. GL_TEXTURE* ], slots = texture units except they are 0-indexed
        unsigned int currentSlotIdx;

        std::map<unsigned int, TextureAndKey> slotsAllocated;
        std::unordered_map<int64_t, unsigned int> slotsAllocatedTbl;
        std::vector<unsigned int> slotsRecycled;

    public:
        static int SLOTS_MAX;

        TextureManager();
        ~TextureManager();

        /** Bind the texture to any free slot. */
        unsigned int bind_texture(const std::weak_ptr<Texture> &tex, const GLuint keySalt);
        /** Unbinds the texture at given slot. */
        void unbind_texture(unsigned int slot);
        /** Resets internal state. Recommended to call this at the start of a rendering function. */
        void clear();
        /** Returns number of currently bound textures. */
        unsigned int get_allocated_num() const;
        
        /** Initializes static values. Must be called after GL context has been initialized! */
        static void init();
    };

    // Helper functions
    namespace texhelper {
        /** Loads 2D texture from file into given texture. */
        void texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride = 0);
    }
}

#endif