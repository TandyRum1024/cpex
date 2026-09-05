/**
 * gfx::uniform - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#include <gfx/uniform.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/gtc/type_ptr.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

std::string Uniform::get_name() {
    return name;
}

// And now for few pre-defined uniform type specializations

void UniformSampler2D::set_tex_slot(GLenum texSlot) {
    this->texSlot = texSlot;
}
void UniformSampler2D::set_tex_filter(GLint texFilterMode) {
    this->texFilterMode = texFilterMode;
}
void UniformSampler2D::set_tex_wrap(GLint texWrapMode) {
    this->texWrapMode = texWrapMode;
}
void UniformSampler2D::apply_uniform(GLint location) {
    val->bind(texSlot + GL_TEXTURE0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texFilterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texFilterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, texWrapMode);
    glUniform1i(location, texSlot);
}

template <>
void UniformTemplated<glm::vec2>::apply_uniform(GLint location) {
    glUniform2fv(location, 1, glm::value_ptr(val));
}
template <>
void UniformVec3::apply_uniform(GLint location) {
    glUniform3fv(location, 1, glm::value_ptr(val));
}
template <>
void UniformVec4::apply_uniform(GLint location) {
    glUniform4fv(location, 1, glm::value_ptr(val));
}

template <>
void UniformIvec2::apply_uniform(GLint location) {
    glUniform2iv(location, 1, glm::value_ptr(val));
}
template <>
void UniformIvec3::apply_uniform(GLint location) {
    glUniform3iv(location, 1, glm::value_ptr(val));
}
template <>
void UniformIvec4::apply_uniform(GLint location) {
    glUniform4iv(location, 1, glm::value_ptr(val));
}

template <>
void UniformBvec2::apply_uniform(GLint location) {
    glUniform2iv(location, 1, glm::value_ptr(static_cast<glm::ivec2>(val)));
}
template <>
void UniformBvec3::apply_uniform(GLint location) {
    glUniform3iv(location, 1, glm::value_ptr(static_cast<glm::ivec3>(val)));
}
template <>
void UniformBvec4::apply_uniform(GLint location) {
    glUniform4iv(location, 1, glm::value_ptr(static_cast<glm::ivec4>(val)));
}

template <>
void UniformUvec2::apply_uniform(GLint location) {
    glUniform2uiv(location, 1, glm::value_ptr(val));
}
template <>
void UniformUvec3::apply_uniform(GLint location) {
    glUniform3uiv(location, 1, glm::value_ptr(val));
}
template <>
void UniformUvec4::apply_uniform(GLint location) {
    glUniform4uiv(location, 1, glm::value_ptr(val));
}

template <>
void UniformBool::apply_uniform(GLint location) {
    glUniform1i(location, static_cast<GLint>(val));
}
template <>
void UniformInt::apply_uniform(GLint location) {
    glUniform1i(location, val);
}
template <>
void UniformFloat::apply_uniform(GLint location) {
    glUniform1f(location, val);
}
template <>
void UniformUint::apply_uniform(GLint location) {
    glUniform1ui(location, val);
}

template <>
void UniformMat3::apply_uniform(GLint location) {
    glUniformMatrix3fv(location, 1, false, glm::value_ptr(val));
}
template <>
void UniformMat4::apply_uniform(GLint location) {
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(val));
}
