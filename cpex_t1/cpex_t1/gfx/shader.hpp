/**
 * gfx::shader - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_SHD_GUARD
#define __CPEX_GFX_SHD_GUARD
#include <string>
#include <map>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace gfx {
    /** Contains all the neccessary informations to compile & use shaders. */
    class Shader {
        std::shared_ptr<spdlog::logger> _logger;

        std::string name;
        GLuint shaderProgram = 0;
        
        /** Indexed by shader type (`GL_VERTEX_SHADER`, etc), contains results of `glCreateShader()` */
        std::map<GLenum, GLuint> loadedShaders;
        
        /** (DEBUG) Table for converting OpenGL's shader type to human readable names. */
        static const std::map<GLenum, std::string> TBL_SHADER_TYPE_TO_NAME;
        /** Converts shader type (`GL_VERTEX_SHADER`, etc) into human readable names. */
        std::string get_shader_type_name(GLenum type);
        /** Release aquired resources. */
        void release_resources();

    public:
        Shader():
            name(""),
            shaderProgram(0) {
            _logger = spdlog::get("GFX::SHADER");
            if (!_logger) {
                _logger = spdlog::stdout_color_mt("GFX::SHADER");
                // _logger->set_level(spdlog::level::debug);
            }
        }
        Shader(std::string name):
            name(name),
            shaderProgram(0) {
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
            return *this;
        }

        /** Loads vertex/fragment/etc shader from given file path. */
        void load_shader_from(std::string filePath, GLenum type);
        /** Sets vertex/fragment/etc shdaer from given string source code. */
        void set_shader(std::string src, GLenum type);
        /** Links currently set shaders into a new program. */
        void link_program();
        /** Uses shader for next rendering. */
        void use_shader();
    };
}
#endif