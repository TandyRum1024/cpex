/**
 * gfx::uniform - OpenGL Shader abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_UNIFORM_GUARD
#define __CPEX_GFX_UNIFORM_GUARD

#include <string>

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
        std::string name;
        
    public:
        Uniform(std::string name):
            name(name) {};
        virtual ~Uniform() = default;
        
        /** Apply uniform value to currently using shader at given location. */
        virtual void apply_uniform(GLint location) = 0;

        /** Set the name. */
        std::string get_name();
    };

    /** Base uniform abstraction with type attatched to it. */
    template <typename T>
    class UniformTemplated: public Uniform {
    protected:
        T val;

    public:
        UniformTemplated(std::string name):
            Uniform(name),
            val({}) {};
        UniformTemplated(std::string name, T val):
            Uniform(name),
            val(val) {};
        virtual ~UniformTemplated() = default;
        
        void apply_uniform(GLint location) override;
        /** Set the value. */
        void set_value(T val);
        /** Set the value. */
        void operator=(T val);
    };

    // And now for few pre-defined uniform type specializations
    /** sampler2D uniform. */
    class UniformSampler2D: public UniformTemplated<GLuint> {
    protected:
        GLenum texSlot;
        GLint texFilterMode;
        GLint texWrapMode;
        // GLuint texId;
    
    public:
        UniformSampler2D(std::string name):
            UniformTemplated(name, 0),
            texSlot(0),
            texFilterMode(GL_LINEAR),
            texWrapMode(GL_CLAMP_TO_EDGE) {};
        UniformSampler2D(std::string name, GLuint val):
            UniformTemplated(name, val),
            texSlot(0),
            texFilterMode(GL_LINEAR),
            texWrapMode(GL_CLAMP_TO_EDGE) {};
        UniformSampler2D(std::string name, GLuint val, GLint texFilterMode, GLint texWrapMode):
            UniformTemplated(name, val),
            texSlot(0),
            texFilterMode(texFilterMode),
            texWrapMode(texWrapMode) {};
        
        void apply_uniform(GLint location) override;
        void set_tex_slot(GLenum texSlot);
        void set_tex_filter(GLint texFilterMode);
        void set_tex_wrap(GLint texWrapMode);
    };
    /** vec2 uniform. */
    using UniformVec2 = UniformTemplated<glm::vec2>; // = `typedef UniformTemplated<glm::vec2> UniformVec2;`
    /** vec3 uniform. */
    using UniformVec3 = UniformTemplated<glm::vec3>;
    /** vec4 uniform. */
    using UniformVec4 = UniformTemplated<glm::vec4>;
    
    /** ivec2 uniform. */
    using UniformIvec2 = UniformTemplated<glm::ivec2>;
    /** ivec3 uniform. */
    using UniformIvec3 = UniformTemplated<glm::ivec3>;
    /** ivec4 uniform. */
    using UniformIvec4 = UniformTemplated<glm::ivec4>;

    /** bvec2 uniform. */
    using UniformBvec2 = UniformTemplated<glm::bvec2>;
    /** bvec3 uniform. */
    using UniformBvec3 = UniformTemplated<glm::bvec3>;
    /** bvec4 uniform. */
    using UniformBvec4 = UniformTemplated<glm::bvec4>;

    /** uvec2 uniform. */
    using UniformUvec2 = UniformTemplated<glm::uvec2>;
    /** uvec3 uniform. */
    using UniformUvec3 = UniformTemplated<glm::uvec3>;
    /** uvec4 uniform. */
    using UniformUvec4 = UniformTemplated<glm::uvec4>;

    /** bool uniform. */
    using UniformBool = UniformTemplated<bool>;
    /** int uniform. */
    using UniformInt = UniformTemplated<int>;
    /** float uniform. */
    using UniformFloat = UniformTemplated<float>;
    /** uint uniform. */
    using UniformUint = UniformTemplated<unsigned int>;

    /** mat3 uniform. */
    using UniformMat3 = UniformTemplated<glm::mat3>;
    /** mat4 uniform. */
    using UniformMat4 = UniformTemplated<glm::mat4>;
    
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    void UniformTemplated<T>::set_value(T val) {
        this->val = val;
    }

    template <typename T>
    void UniformTemplated<T>::operator=(T val) {
        set_value(val);
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif