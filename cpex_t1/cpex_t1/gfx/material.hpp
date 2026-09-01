/**
 * gfx::material - Shader & uniform etc. abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MTL_GUARD
#define __CPEX_GFX_MTL_GUARD

#include <gfx/shader.hpp>
#include <gfx/uniform.hpp>

namespace gfx {
    /** Contains set of uniforms and associated shader. */
    class Material {
        std::shared_ptr<Shader> shd;
        std::vector<std::shared_ptr<Uniform>> uniforms;
        std::vector<GLint> uniformLocations;

    public:
        /** Links shader to this material. */
        void set_shader(std::shared_ptr<Shader> shd);
        /** Applies shader and uniform for next render. */
        void apply_material();
        /** Adds an uniform. */
        template <typename T>
        void add_uniform(T &uniform);
        /** Returns an uniform with given name and type. `nullptr` if not found or wrong type. */
        template <typename T>
        std::shared_ptr<T> get_uniform(std::string name);
    };

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
    template <typename T>
    void Material::add_uniform(T &uniform) {
        GLint location = 0;
        uniforms.push_back(std::make_shared<T>(uniform));

        if (shd) {
            location = shd->get_uniform_location(uniform.get_name());
        }
        uniformLocations.push_back(location);
    }

    template <typename T>
    std::shared_ptr<T> Material::get_uniform(std::string name) {
        auto res = std::find_if(
            uniforms.begin(),
            uniforms.end(),
            [name](const std::shared_ptr<Uniform> &uniform) {
                return uniform ? (uniform->get_name() ==  name) : false;
            }
        );
        std::shared_ptr<Uniform> resUni = *res;

        // use `dynamic_pointer_cast` instead of normal `dynamic_cast` for `shared_ptr`!
        return (res != uniforms.end()) ? std::dynamic_pointer_cast<T>(resUni) : nullptr;
    }
    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif