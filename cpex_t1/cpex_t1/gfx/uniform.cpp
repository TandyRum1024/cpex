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
    samplerId(0),
    texTarget(val ? val->get_target() : GL_TEXTURE_2D),
    texSlot(val ? val->get_bound_unit() : GL_TEXTURE0),
    keySalt(0),
    texFilterMode(GL_LINEAR),
    texWrapMode(GL_REPEAT) {
        assert((texFilterMode == GL_NEAREST || texFilterMode == GL_LINEAR) && "texFilterMode UNSUPPORTED VALUE");
        assert((
            texWrapMode == GL_CLAMP_TO_EDGE
            || texWrapMode == GL_MIRRORED_REPEAT
            || texWrapMode ==  GL_CLAMP_TO_BORDER
            || texWrapMode ==  GL_REPEAT
        ) && "texWrapMode UNSUPPORTED VALUE");

        glGenSamplers(1, &samplerId);
        set_tex_filter(texFilterMode);
        set_tex_wrap(texWrapMode);
    }

UniformSampler::~UniformSampler() {
    if (samplerId) {
        glDeleteSamplers(1, &samplerId);
        // No need to unbind samplers to texture unit, as they are automatically reset to 0 once deleted
    }
}

UniformSampler::UniformSampler(UniformSampler &&other):
    UniformTemplated(std::move(other)),
    samplerId(other.samplerId),
    texTarget(other.texTarget),
    texSlot(other.texSlot),
    keySalt(0),
    texFilterMode(GL_LINEAR),
    texWrapMode(GL_REPEAT) {
        // Try to swap samplerId to invalid one...
        other.samplerId = 0;
        set_tex_filter(other.texFilterMode);
        set_tex_wrap(other.texWrapMode);
    }
UniformSampler& UniformSampler::operator=(UniformSampler &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }
    
    std::swap(texTarget, other.texTarget);
    std::swap(texSlot, other.texSlot);
    std::swap(keySalt, other.keySalt);
    // std::swap(texFilterMode, other.texFilterMode);
    // std::swap(texWrapMode, other.texWrapMode);
    set_tex_filter(other.texFilterMode);
    set_tex_wrap(other.texWrapMode);
    samplerId = other.samplerId;
    other.samplerId = 0;

    std::swap(val, other.val);
    // UniformTemplated::operator=(std::move(other));

    return *this;
}

void UniformSampler::update_key_salt() {
    char keyTexFilterMode;
    char keyTexWrapMode;

    switch (texFilterMode) {
        case GL_NEAREST:
            keyTexFilterMode = 0b0;
            break;
        default: // GL_LINEAR
            keyTexFilterMode = 0b1;
            break;
    }

    //texWrapMode == GL_CLAMP_TO_EDGE
    // || texWrapMode == GL_MIRRORED_REPEAT
    // || texWrapMode ==  GL_MIRROR_CLAMP_TO_EDGE
    // || texWrapMode ==  GL_REPEAT
    switch (texWrapMode) {
        case GL_CLAMP_TO_EDGE:
            keyTexWrapMode = 0b00;
            break;
        case GL_MIRRORED_REPEAT:
            keyTexWrapMode = 0b01;
            break;
        case GL_MIRROR_CLAMP_TO_EDGE:
            keyTexWrapMode = 0b10;
            break;
        default: // GL_REPEAT
            keyTexWrapMode = 0b11;
            break;
    }

    keySalt = (keyTexFilterMode << 0)
            | (keyTexWrapMode << 1);
}

uint8_t UniformSampler::get_key_salt() const {
    return keySalt;
}

void UniformSampler::set_value(std::shared_ptr<Texture> val) {
    UniformTemplated::set_value(val);
    this->texTarget = val->get_target();
}
void UniformSampler::set_tex_filter(GLint texFilterMode) {
    this->texFilterMode = texFilterMode;
    // `glSamplerParameter*` uses same params as `glTexParameter*`...
    glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, texFilterMode);
    glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, texFilterMode);
    update_key_salt();
}
void UniformSampler::set_tex_wrap(GLint texWrapMode) {
    this->texWrapMode = texWrapMode;
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, texWrapMode);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, texWrapMode);
    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_R, texWrapMode);
    update_key_salt();
}
void UniformSampler::set_tex_target(GLenum texTarget) {
    this->texTarget = texTarget;
}
void UniformSampler::set_tex_slot(GLuint texSlot) {
    // zcl::logger("GFX")->warn("SAMPLER {} SLOT {} BIND TO SAMPLER {}", name, texSlot - GL_TEXTURE0, samplerId);
    this->texSlot = texSlot;
    glBindSampler(texSlot, samplerId);
}
void UniformSampler::apply_uniform(GLint location) const {
    // zcl::logger("GFX")->warn("SAMPLER {} SLOT {} / {} & {}", name, texSlot - GL_TEXTURE0, texFilterMode, texWrapMode);
    glUniform1i(location, texSlot); // be sure to convert from range [GL_TEXTURE0 ..] to [0 ..]
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