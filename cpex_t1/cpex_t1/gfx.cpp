#include <string>
#include <iostream>

#include <gfx.hpp>
#include <zcl/zcl.hpp>

using namespace gfx;

Shader::Shader(std::string vertFilePath, std::string fragFilePath)
    : vertShaderFilePath(vertFilePath)
    , fragShaderFilePath(fragFilePath) {
    load();
}

Shader::~Shader() {
    if (vertShader) {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader) {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}

void Shader::load() {
    if (vertShader) {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader) {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();

    if (!vertShader || !fragShader) {
        throw std::runtime_error("Failed to glCreateShader()!");
    }
    
    std::cout << "[GFX] Loading from (vert: " << vertShaderFilePath << ", frag: " << fragShaderFilePath << " )" << std::endl;

    vertShaderSrc = zcl::file::read_file_to_string(vertShaderFilePath);
    fragShaderSrc = zcl::file::read_file_to_string(fragShaderFilePath);

    auto vertShaderCstr = vertShaderSrc.c_str();
    auto fradShaderCstr = fragShaderSrc.c_str();
    int compileRes;

    glShaderSource(vertShader, 1, &vertShaderCstr, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileRes);

    if (!compileRes) {
        char msg[512];
        glGetShaderInfoLog(vertShader, 512, NULL, msg);
        throw std::runtime_error(std::string("Failed to compile vertex shader:\n") + msg);
    }

    glShaderSource(fragShader, 1, &fradShaderCstr, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compileRes);

    if (!compileRes) {
        char msg[512];
        glGetShaderInfoLog(fragShader, 512, NULL, msg);
        throw std::runtime_error(std::string("Failed to compile fragment shader:\n") + msg);
    }

    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &compileRes);

    if (!compileRes) {
        char msg[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, msg);
        throw std::runtime_error(std::string("Failed to link shader program:\n") + msg);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    vertShader = 0;
    fragShader = 0;
}

void Shader::load_from(std::string vertFilePath, std::string fragFilePath) {
    fragShaderFilePath = fragFilePath;
    vertShaderFilePath = vertFilePath;
    load();
}

void Shader::load_from(std::string baseFilePath) {
    load_from(baseFilePath + ".vert", baseFilePath + ".frag");
}

void Shader::set() {
    if (shaderProgram) {
        glUseProgram(shaderProgram);
    }
    else {
        std::cerr << "[GFX] Shader is not ready!" << std::endl;
    }
}