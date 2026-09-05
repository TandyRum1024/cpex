/**
 * gfx::texture - OpenGL Texture abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_TEX_GUARD
#define __CPEX_GFX_TEX_GUARD

#include <string>
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

        /** Frees OpenGL resources. */
        void free_resources();
        
        /** Load from given pixel buffer. */
        void load_from_buffer_2d(const void* pixels, int wid, int hei, GLenum dataFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE, int mipmapLevels = 0);
        /** Set OpenGL texture target and format. */
        void set_format(GLint internalFormat = GL_RGBA8);
        
        /** Bind this texture to given slot. */
        void bind(GLenum slot);
        /** Set OpenGL texture parameter. MUST be called after `bind()`! */
        void set_texture_param(GLint texFilterMode, GLint texWrapMode);
    };

    // Helper functions
    /** Loads 2D texture from file into given texture. */
    void texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride = 0);
}

#endif