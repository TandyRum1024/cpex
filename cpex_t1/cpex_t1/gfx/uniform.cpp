/**
 * gfx::uniform - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#include <gfx/uniform.hpp>

using namespace gfx;

std::string Uniform::get_name() {
    return name;
}

void UniformVec4::set_value(GLfloat vec[4]) {
    std::copy(vec, vec + 4, this->vec);
}

void UniformVec4::set_value(std::initializer_list<GLfloat> vec) {
    std::copy(vec.begin(), vec.begin() + 4, this->vec);
}

void UniformVec4::operator=(GLfloat vec[4]) {
    set_value(vec);
}

void UniformVec4::apply_uniform(GLint location) {
    glUniform4fv(location, 1, vec);
}