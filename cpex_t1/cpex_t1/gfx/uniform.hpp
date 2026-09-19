/**
 * gfx::uniform - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_UNIFORM_GUARD
#define __CPEX_GFX_UNIFORM_GUARD

#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

#include <zcl/zcl.hpp>
#include <gfx/texture.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Base uniform abstraction. */
    class Uniform {
    protected:
        const std::string name;
        
    public:
        Uniform(const std::string &name);
        virtual ~Uniform() = default;
        
        /** Apply uniform value to currently using shader at given location. */
        virtual void apply_uniform(GLint location) const = 0;

        /** Get the name. */
        std::string get_name() const;
    };

    /** Base uniform abstraction with type attatched to it. */
    template <typename V>
    class UniformTemplated: public Uniform {
    protected:
        V val;

    public:
        UniformTemplated(const std::string &name);
        UniformTemplated(const std::string &name, V val);
        virtual ~UniformTemplated() = default;
        
        virtual void apply_uniform(GLint location) const override;
        /** Set the value. */
        void set_value(V val);
        /** Set the value. */
        void operator=(V val);
    };

    // And now for few pre-defined uniform type specializations
    /** sampler2D uniform. */
    class UniformSampler2D: public UniformTemplated<std::shared_ptr<Texture>> {
    protected:
        GLenum texSlot;
        GLint texFilterMode;
        GLint texWrapMode;
    
    public:
        UniformSampler2D(const std::string &name);
        UniformSampler2D(const std::string &name, std::shared_ptr<Texture> val);
        UniformSampler2D(const std::string &name, std::shared_ptr<Texture> val, GLint texFilterMode, GLint texWrapMode);
        
        void apply_uniform(GLint location) const override;
        void set_tex_slot(GLenum texSlot);
        void set_tex_filter(GLint texFilterMode);
        void set_tex_wrap(GLint texWrapMode);
    };

    /** vec2 uniform. */
    using UniformVec2 = UniformTemplated<glm::vec2>; // = `typedef UniformTemplated<glm::vec2> UniformVec2;`
    // (forward decl. here so that other automatically generated templates doesn't clash with this!!)
    template <>
    void UniformVec2::apply_uniform(GLint location) const;
    /** vec3 uniform. */
    using UniformVec3 = UniformTemplated<glm::vec3>;
    template <>
    void UniformVec3::apply_uniform(GLint location) const;
    /** vec4 uniform. */
    using UniformVec4 = UniformTemplated<glm::vec4>;
    template <>
    void UniformVec4::apply_uniform(GLint location) const;

    /** ivec2 uniform. */
    using UniformIvec2 = UniformTemplated<glm::ivec2>;
    template <>
    void UniformIvec2::apply_uniform(GLint location) const;
    /** ivec3 uniform. */
    using UniformIvec3 = UniformTemplated<glm::ivec3> const;
    template <>
    void UniformIvec3::apply_uniform(GLint location) const;
    /** ivec4 uniform. */
    using UniformIvec4 = UniformTemplated<glm::ivec4>;
    template <>
    void UniformIvec4::apply_uniform(GLint location) const;

    /** bvec2 uniform. */
    using UniformBvec2 = UniformTemplated<glm::bvec2>;
    template <>
    void UniformBvec2::apply_uniform(GLint location) const;
    /** bvec3 uniform. */
    using UniformBvec3 = UniformTemplated<glm::bvec3>;
    template <>
    void UniformBvec3::apply_uniform(GLint location) const;
    /** bvec4 uniform. */
    using UniformBvec4 = UniformTemplated<glm::bvec4>;
    template <>
    void UniformBvec4::apply_uniform(GLint location) const;

    /** uvec2 uniform. */
    using UniformUvec2 = UniformTemplated<glm::uvec2>;
    template <>
    void UniformUvec2::apply_uniform(GLint location) const;
    /** uvec3 uniform. */
    using UniformUvec3 = UniformTemplated<glm::uvec3>;
    template <>
    void UniformUvec3::apply_uniform(GLint location) const;
    /** uvec4 uniform. */
    using UniformUvec4 = UniformTemplated<glm::uvec4>;
    template <>
    void UniformUvec4::apply_uniform(GLint location) const;

    /** bool uniform. */
    using UniformBool = UniformTemplated<bool>;
    template <>
    void UniformBool::apply_uniform(GLint location) const;
    /** int uniform. */
    using UniformInt = UniformTemplated<int>;
    template <>
    void UniformInt::apply_uniform(GLint location) const;
    /** float uniform. */
    using UniformFloat = UniformTemplated<float>;
    template <>
    void UniformFloat::apply_uniform(GLint location) const;
    /** uint uniform. */
    using UniformUint = UniformTemplated<unsigned int>;
    template <>
    void UniformUint::apply_uniform(GLint location) const;

    /** mat3 uniform. */
    using UniformMat3 = UniformTemplated<glm::mat3>;
    template <>
    void UniformMat3::apply_uniform(GLint location) const;
    /** mat4 uniform. */
    using UniformMat4 = UniformTemplated<glm::mat4>;
    template <>
    void UniformMat4::apply_uniform(GLint location) const;
    
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    UniformTemplated<T>::UniformTemplated(const std::string &name):
        UniformTemplated(name, {}) {}
    
    template <typename T>
    UniformTemplated<T>::UniformTemplated(const std::string &name, T val):
        Uniform(name),
        val(val) {}
    
    template <typename T>
    void UniformTemplated<T>::set_value(T val) {
        this->val = val;
    }

    template <typename T>
    void UniformTemplated<T>::operator=(T val) {
        set_value(val);
    }

    template <typename T>
    void UniformTemplated<T>::apply_uniform(GLint location) const {
        throw std::runtime_error("apply_uniform() not implemented!");
    }
    
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif