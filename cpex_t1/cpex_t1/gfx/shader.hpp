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
        
        /** Release aquired resources. */
        void release_resources();
        
        /** (DEBUG) Table for converting OpenGL's shader type to human readable names. */
        static const std::map<GLenum, std::string> TBL_SHADER_TYPE_TO_NAME;
        /** Converts shader type (`GL_VERTEX_SHADER`, etc) into human readable names. */
        std::string get_shader_type_name(GLenum type);

    public:
        Shader():
            name(""),
            shaderProgram(0),
            _logger(zcl::logger("GFX::SHADER"))
            {}
        Shader(std::string name):
            name(name),
            shaderProgram(0),
            _logger(zcl::logger("GFX::SHADER"))
            {
            // std::cout << "[GFX] Shader `" << name << "` created!" << std::endl;
        }
        ~Shader() {
            // std::cout << "[GFX] Shader `" << name << "` destroyed!" << std::endl;
            release_resources();
        }

        Shader(const Shader &other) = delete; // (RAII) Disable copy
        Shader& operator=(const Shader &other) = delete; // (RAII) Disable copy

        Shader(Shader &&other): // (RAII) Move
            loadedShaders(std::move(other.loadedShaders)),
            name(std::exchange(other.name, "_MOVED")),
            _logger(std::move(other._logger)),
            // (replace GL resources with dummy)
            shaderProgram(std::exchange(other.shaderProgram, 0))
            {
            // std::cout << "[GFX] Shader `" << name << "` moved!" << std::endl;
        }
        Shader& operator=(Shader &&other) { // (RAII) Move
            if (this == &other) {
                // Self assignment, no need to move
                return *this;
            }

            // std::cout << "[GFX] Shader `" << name << "` <- `" << other.name << "` moved!" << std::endl;
            release_resources();
            
            std::swap(loadedShaders, other.loadedShaders);
            std::swap(shaderProgram, other.shaderProgram);
            std::swap(name, other.name);
            std::swap(_logger, other._logger);
            return *this;
        }

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