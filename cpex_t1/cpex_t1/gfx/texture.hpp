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

        // Format
        GLenum fmtTarget;
        GLint fmtInternal;
        
    public:
        Texture();
        Texture(std::string name);
        ~Texture();
        
        // Disable default copy ops, since `Texture` is move only & frees resource on destruction!

        Texture(const Texture &other) = delete;
        Texture& operator=(const Texture &other) = delete;

        // Only implement move ops for now

        Texture(Texture &&other) = default;
        Texture& operator=(Texture &&other);

        void free_resources();
        
        void load_from_buffer_2d(const void* pixels, int wid, int hei, GLenum dataFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE, int mipmapLevels = 0);
        void set_format(GLenum target = GL_TEXTURE_2D, GLint internalFormat = GL_RGBA8);

        void bind(GLenum slot);
    };

    // Helper functions
    /** Loads 2D texture from file into given texture. */
    void texture_load_from_file_2d(Texture& tex, std::filesystem::path file, GLint formatOverride = 0);
}

#endif