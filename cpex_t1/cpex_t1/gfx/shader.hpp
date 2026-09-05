/**
 * gfx::shader - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_SHD_GUARD
#define __CPEX_GFX_SHD_GUARD

#include <memory>
#include <algorithm>
#include <string>
#include <map>
#include <filesystem>

// LIBRARIES //
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Contains all the neccessary informations to compile & use shaders. */
    class Shader {
        std::shared_ptr<spdlog::logger> _logger;

        std::string name;
        GLuint shaderProgram = 0;
        
        /** Indexed by shader type (`GL_VERTEX_SHADER`, etc), contains results of `glCreateShader()` */
        std::map<GLenum, GLuint> loadedShaders;
        
        /** Releases OpenGL resources. */
        void release_resources();
        
        /** (DEBUG) Table for converting OpenGL's shader type to human readable names. */
        static const std::map<GLenum, std::string> TBL_SHADER_TYPE_TO_NAME;
        /** Converts shader type (`GL_VERTEX_SHADER`, etc) into human readable names. */
        std::string get_shader_type_name(GLenum type);

    public:
        Shader();
        Shader(std::string name);
        ~Shader();

        // Disable default copy ops, since `Shader` is move only (tied to OpenGL objects that are hard to copy)!

        Shader(const Shader &other) = delete;
        Shader& operator=(const Shader &other) = delete;

        // Only implement move ops for now

        Shader(Shader &&other);
        Shader& operator=(Shader &&other);

        /** Loads vertex/fragment/etc shader from given file path. */
        void load_shader_from(std::filesystem::path filePath, GLenum type);
        /** Sets vertex/fragment/etc shdaer from given string source code. */
        void set_shader(std::string src, GLenum type);
        /** Links currently set shaders into a new program. */
        void link_program();
        /** Uses shader for next rendering. */
        void apply_shader();
        /** Returns uniform location. */
        GLint get_uniform_location(std::string name);
    };
}
#endif