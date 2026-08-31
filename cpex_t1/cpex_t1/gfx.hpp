#ifndef __CPEX_GFX_GURD
#define __CPEX_GFX_GURD
#include <string>
#include <map>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace gfx {
    class Shader {
        // (DEBUG) Table for converting OpenGL's shader type to human readable names
        static const std::map<GLenum, std::string> TBL_SHADER_TYPE_TO_NAME;

        std::string name;
        std::map<GLenum, GLuint> loadedShaders;
        GLuint shaderProgram;

        std::string get_shader_type_name(GLenum type);

    public:
        Shader(std::string name);
        ~Shader();

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