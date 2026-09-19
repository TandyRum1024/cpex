/**
 * gfx::material - Shader & uniform etc. abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MTL_GUARD
#define __CPEX_GFX_MTL_GUARD

#include <gfx/shader.hpp>
#include <gfx/uniformset.hpp>

namespace gfx {
    /** Contains set of uniforms and associated shader. */
    class Material {
        std::shared_ptr<Shader> shd;
        UniformSet uniforms;
        std::vector<GLint> uniformLocations;

    public:
        /** Links shader to this material. */
        void set_shader(std::shared_ptr<Shader> shd);
        /** Applies shader and uniform for next render. */
        void apply_material();
        /** Adds an uniform. */
        template <typename T>
        void add_uniform(const T &uniform);
        /** Adds uniforms. */
        template <typename...T>
        void add_uniforms(const T... uniform);
        /** Returns an uniform with given name and type. `nullptr` if not found or wrong type. */
        template <typename T>
        std::shared_ptr<T> get_uniform(const std::string name);
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    void Material::add_uniform(const T &uniform) {
        GLint location = 0;
        uniforms.add_uniform(uniform);

        if (shd) {
            location = shd->get_uniform_location(uniform.get_name());
        }
        uniformLocations.push_back(location);
    }

    template <typename...T>
    void Material::add_uniforms(const T... uniform) {
        (add_uniform<T>(uniform), ...);
    }

    template <typename T>
    std::shared_ptr<T> Material::get_uniform(const std::string name) {
        return uniforms.get_uniform<T>(name);
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif