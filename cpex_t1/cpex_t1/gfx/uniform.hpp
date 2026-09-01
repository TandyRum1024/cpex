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

    /** Vec4 (`Float4`) uniform. */
    class UniformVec4: public Uniform {
        GLfloat vec[4] = { 0.0, 0.0, 0.0, 0.0 };

    public:
        UniformVec4(std::string name, GLfloat vec[4]):
            Uniform(name),
            vec()
            {
            std::copy(vec, vec + 4, this->vec);
        }
        UniformVec4(std::string name, std::initializer_list<GLfloat> vec):
            Uniform(name),
            vec()
            {
            std::copy(vec.begin(), vec.begin() + 4, this->vec);
        }
        UniformVec4(std::string name):
            Uniform(name),
            vec{0.0, 0.0, 0.0, 0.0} {}

        /** Set the value. */
        void set_value(GLfloat vec[4]);
        void set_value(std::initializer_list<GLfloat> vec);
        /** Set the value. */
        void operator=(GLfloat vec[4]);

        void apply_uniform(GLint location) override;
    };
}
#endif