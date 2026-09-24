/**
 * gfx::uniform - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#include <gfx/uniform.hpp>
#define GLM_ENABLE_EXPERIMENTAL

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

Uniform::Uniform(const std::string &name):
    name(name) {
        if (name.length() <= 0) {
            zcl::logger("GFX")->error("Uniform name must not be empty!");
            assert(false && "Uniform name must not be empty!");
        }
    }

std::string Uniform::get_name() const {
    return name;
}

// And now for few pre-defined uniform type specializations
UniformSampler::UniformSampler(const std::string &name):
    UniformSampler(name, nullptr, GL_LINEAR, GL_CLAMP_TO_EDGE) {}
UniformSampler::UniformSampler(const std::string &name, std::shared_ptr<Texture> val):
    UniformSampler(name, std::move(val), GL_LINEAR, GL_CLAMP_TO_EDGE) {}
UniformSampler::UniformSampler(const std::string &name, std::shared_ptr<Texture> val, GLint texFilterMode, GLint texWrapMode):
    UniformTemplated(name, std::move(val)),
    texFilterMode(texFilterMode),
    texWrapMode(texWrapMode) {}

void UniformSampler::set_tex_filter(GLint texFilterMode) {
    this->texFilterMode = texFilterMode;
}
void UniformSampler::set_tex_wrap(GLint texWrapMode) {
    this->texWrapMode = texWrapMode;
}
void UniformSampler::apply_uniform(GLint location) const {
    if (auto slot = val->get_bound_slot(); slot >= GL_TEXTURE0) {
        auto target = val->get_target();
        
        glActiveTexture(slot);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, texFilterMode);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, texFilterMode);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, texWrapMode);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, texWrapMode);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, texWrapMode);
        glUniform1i(location, slot - GL_TEXTURE0); // be sure to convert from range [GL_TEXTURE0 ..] to [0 ..]
    }
}

template <>
void UniformTemplated<glm::vec2>::apply_uniform(GLint location) const {
    glUniform2fv(location, 1, glm::value_ptr(val));
}
template <>
void UniformVec3::apply_uniform(GLint location) const {
    glUniform3fv(location, 1, glm::value_ptr(val));
}
template <>
void UniformVec4::apply_uniform(GLint location) const {
    glUniform4fv(location, 1, glm::value_ptr(val));
}

template <>
void UniformIvec2::apply_uniform(GLint location) const {
    glUniform2iv(location, 1, glm::value_ptr(val));
}
template <>
void UniformIvec3::apply_uniform(GLint location) const {
    glUniform3iv(location, 1, glm::value_ptr(val));
}
template <>
void UniformIvec4::apply_uniform(GLint location) const {
    glUniform4iv(location, 1, glm::value_ptr(val));
}

template <>
void UniformBvec2::apply_uniform(GLint location) const {
    glUniform2iv(location, 1, glm::value_ptr(static_cast<glm::ivec2>(val)));
}
template <>
void UniformBvec3::apply_uniform(GLint location) const {
    glUniform3iv(location, 1, glm::value_ptr(static_cast<glm::ivec3>(val)));
}
template <>
void UniformBvec4::apply_uniform(GLint location) const {
    glUniform4iv(location, 1, glm::value_ptr(static_cast<glm::ivec4>(val)));
}

template <>
void UniformUvec2::apply_uniform(GLint location) const {
    glUniform2uiv(location, 1, glm::value_ptr(val));
}
template <>
void UniformUvec3::apply_uniform(GLint location) const {
    glUniform3uiv(location, 1, glm::value_ptr(val));
}
template <>
void UniformUvec4::apply_uniform(GLint location) const {
    glUniform4uiv(location, 1, glm::value_ptr(val));
}

template <>
void UniformBool::apply_uniform(GLint location) const {
    glUniform1i(location, static_cast<GLint>(val));
}
template <>
void UniformInt::apply_uniform(GLint location) const {
    glUniform1i(location, val);
}
template <>
void UniformFloat::apply_uniform(GLint location) const {
    glUniform1f(location, val);
}
template <>
void UniformUint::apply_uniform(GLint location) const {
    glUniform1ui(location, val);
}

template <>
void UniformMat3::apply_uniform(GLint location) const {
    glUniformMatrix3fv(location, 1, false, glm::value_ptr(val));
}
template <>
void UniformMat4::apply_uniform(GLint location) const {
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(val));
}

#undef GLM_ENABLE_EXPERIMENTAL