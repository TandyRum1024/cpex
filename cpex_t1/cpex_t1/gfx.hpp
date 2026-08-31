#ifndef __CPEX_GFX_GURD
#define __CPEX_GFX_GURD
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace gfx {
    class Shader {
        std::string fragShaderFilePath;
        std::string vertShaderFilePath;
        std::string vertShaderSrc;
        std::string fragShaderSrc;

        GLuint vertShader;
        GLuint fragShader;
        GLuint shaderProgram;

    public:
        Shader() {}
        Shader(std::string baseFilePath);
        Shader(std::string vertFilePath, std::string fragFilePath);
        ~Shader();

        void load();
        void load_from(std::string baseFilePath);
        void load_from(std::string vertFilePath, std::string fragFilePath);
        void set();
    };
}
#endif