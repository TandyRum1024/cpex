/**
 * gfx::uniformset - Shader uniform collection for material uniform definitions & overrides
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_UNIFORMSET_GUARD
#define __CPEX_GFX_UNIFORMSET_GUARD

#include <string>
#include <memory>

#include <gfx/uniform.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Collection of uniforms. */
    class UniformSet {
        GLuint texSlotIdx = 0;
        std::vector<std::shared_ptr<Uniform>> uniforms;
        
        /** Updates an uniform. */
        void update_texture_slots();

    public:
        /** Adds an uniform. */
        template <typename T>
        void add_uniform(const T &uniform);
        /** Adds uniforms. */
        template <typename...T>
        void add_uniforms(const T... uniform);
        /** Returns an uniform with given name and type. `nullptr` if not found or wrong type. */
        template <typename T>
        std::shared_ptr<T> get_uniform(const std::string name);

        /** Gets uniform at certain position. */
        std::shared_ptr<Uniform>& operator[](int pos);
        /** Number of uniforms. */
        size_t size();
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    void UniformSet::add_uniform(const T &uniform) {
        uniforms.push_back(std::make_shared<T>(uniform));
        update_texture_slots();
    }

    template <typename...T>
    void UniformSet::add_uniforms(const T... uniform) {
        (add_uniform<T>(uniform), ...);
    }

    template <typename T>
    std::shared_ptr<T> UniformSet::get_uniform(const std::string name) {
        auto res = std::find_if(
            uniforms.begin(),
            uniforms.end(),
            [name](const std::shared_ptr<Uniform> &uniform) {
                return uniform ? (uniform->get_name() ==  name) : false;
            }
        );
        std::shared_ptr<Uniform> resUni = (res != uniforms.end()) ? (*res) : nullptr;

        // use `dynamic_pointer_cast` instead of normal `dynamic_cast` for `shared_ptr`!
        return (res != uniforms.end()) ? std::dynamic_pointer_cast<T>(resUni) : nullptr;
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}

#endif