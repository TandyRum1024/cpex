/**
 * gfx::shader - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#include <string>
#include <iostream>

#include <gfx/shader.hpp>
#include <zcl/zcl.hpp>

using namespace gfx;
using namespace std::string_literals; // ""s literal

const std::map<GLenum, std::string> Shader::TBL_SHADER_TYPE_TO_NAME = {
    { GL_VERTEX_SHADER, "GL_VERTEX_SHADER" },
    { GL_FRAGMENT_SHADER, "GL_FRAGMENT_SHADER" },
};

Shader::Shader(std::string name): name(name) { }

Shader::~Shader() {
    for (auto &&shader: loadedShaders) {
        if (auto shaderIdx = shader.second) {
            if (shaderProgram) {
                glDetachShader(shaderProgram, shaderIdx);
            }
            glDeleteShader(shaderIdx);
        }
    }
    loadedShaders.clear();

    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}

std::string Shader::get_shader_type_name(GLenum type) {
    auto it = TBL_SHADER_TYPE_TO_NAME.find(type);
    return (it != TBL_SHADER_TYPE_TO_NAME.end()) ? it->second : ("UNKNOWN_TYPE_"s + std::to_string(it->first));
}

void Shader::load_shader_from(std::string filePath, GLenum type) {
    std::cout << "[GFX] Loading shader (" << get_shader_type_name(type) << ") from `" << filePath << "`..." << std::endl;
    set_shader(zcl::file::read_file_to_string(filePath), type);
}

void Shader::set_shader(std::string src, GLenum type) {
    auto cstr = src.c_str();
    int compileRes;
    GLuint shader;

    std::cout << "[GFX] Set shader (" << get_shader_type_name(type) << ") for `" << name << "`..." << std::endl;

    // Detach & unload previous shader if theres any
    shader = loadedShaders[type];
    if (shader) {
        if (shaderProgram) {
            glDetachShader(shaderProgram, shader);
        }
        glDeleteShader(shader);
        std::cout << "[GFX] Detaching shader (" << get_shader_type_name(type) << ") from `" << name << "`..." << std::endl;
    }

    // Prepare shader object
    shader = glCreateShader(type);
    if (!shader) {
        throw std::runtime_error("Failed to glCreateShader()!");
    }
    loadedShaders[type] = shader;

    // Compile shader
    glShaderSource(shader, 1, &cstr, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileRes);
    if (!compileRes) {
        char msg[512];
        glGetShaderInfoLog(shader, 512, NULL, msg);
        throw std::runtime_error("Failed to compile vertex shader:\n"s + msg);
    }
}

void Shader::link_program() {
    int compileRes;

    std::cout << "[GFX] Linking shader program for `" << name << "`..." << std::endl;

    // Prepare shader program object
    if (shaderProgram) {
        // (detach all previous shaders from this program)
        for (auto &&shader: loadedShaders) {
            if (auto shaderIdx = shader.second) {
                glDetachShader(shaderProgram, shaderIdx);
            }
        }
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    shaderProgram = glCreateProgram();
    if (!shaderProgram) {
        throw std::runtime_error("Failed to glCreateProgram()!");
    }

    // Attach all shader & link to program
    for (auto &&shader: loadedShaders) {
        if (auto shaderIdx = shader.second) {
            glAttachShader(shaderProgram, shader.second);
        }
    }
    glLinkProgram(shaderProgram);
    // (sanity check)
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &compileRes);
    if (!compileRes) {
        char msg[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, msg);
        throw std::runtime_error("Failed to link shader program:\n"s + msg);
    }
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &compileRes);
    if (!compileRes) {
        char msg[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, msg);
        throw std::runtime_error("Shader program not available!\n"s + msg);
    }
}

void Shader::use_shader() {
    if (shaderProgram) {
        glUseProgram(shaderProgram);
    }
    else {
        std::cerr << "[GFX] Shader `" << name << "` is not ready!" << std::endl;
    }
}