/**
 * gfx::material - Shader & uniform etc. abstraction
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MTL_GUARD
#define __CPEX_GFX_MTL_GUARD

#include <gfx/shader.hpp>
#include <gfx/uniformset.hpp>
#include <gfx/texture.hpp>

namespace gfx {
    /** Contains set of uniforms and associated shader. */
    class Material {
        const std::string id;

        std::weak_ptr<Material> parent;
        std::vector<std::shared_ptr<Material>> children;
        std::shared_ptr<Shader> shd;

        /** Dirty flag for lazy-recalculating merged material. */
        bool isMergeRequired;
        /** Uniforms that are unique to this material. Not including the ones from parents! */
        UniformSet uniforms;
        /** Calculated uniforms locations. Do not directly modify this unless you know what you're doing!!! */
        std::vector<GLint> uniformLocations;

        // Internal merge / cached result
        /** Merged uniforms from parents. Do not directly modify this unless you know what you're doing!!! */
        UniformSet mergedUniforms;
        /** Merged shader from either this material or parent(s). Do not directly modify this unless you know what you're doing!!! */
        std::shared_ptr<Shader> mergedShd;
        /** List of texture uniforms. Do not directly modify this unless you know what you're doing!!! */
        std::vector<std::shared_ptr<UniformSampler>> mergedUniformSamplers;

        /** Merge from parent materials and this material and cache them. */
        inline void process_merge();
        /** Append all uniforms in this material to given uniformset. */
        void add_uniforms_to(UniformSet &outUniforms);
        
    public:
        Material() = delete;
        Material(const std::string &id);
        Material(const std::string &id, const std::weak_ptr<Material> &parent);

        /** Returns id of this material. */
        const std::string get_id() const;

        /** Adds child to this material. */
        void add_child(const std::shared_ptr<Material> &child);
        /** Applies shader and uniform for next render. */
        void apply_material(TextureManager &texManager);
        /** Links shader to this material. */
        void set_shader(std::shared_ptr<Shader> shd);
        /** Adds an uniform. */
        template <typename T>
        void add_uniform(const T &uniform);
        /** Adds uniforms. */
        template <typename...T>
        void add_uniforms(const T... uniform);
        /** Returns an uniform with given name and type. `nullptr` if not found or wrong type. */
        template <typename T>
        std::shared_ptr<T> get_uniform(const std::string name);
        /** Returns an iterable for accessing the uniformset. */
        const std::vector<std::shared_ptr<Uniform>>& get_uniforms();

        /** Returns whether or not if this material has its properties changed and must be re-processed. */
        bool get_merge_required() const;
        /** Recursively marks this material for re-processing. */
        void set_merge_required();
    };

    /** Returns a new material "inherited" from this material. */
    std::shared_ptr<Material> material_make_inherited(std::shared_ptr<Material> parent, const std::string &childId);

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

    template <typename T>
    void Material::add_uniform(const T &uniform) {
        uniforms.add_uniform(uniform);
        set_merge_required();
    }

    template <typename...T>
    void Material::add_uniforms(const T... uniform) {
        (add_uniform<T>(uniform), ...);
    }

    template <typename T>
    std::shared_ptr<T> Material::get_uniform(const std::string name) {
        process_merge();
        return mergedUniforms.get_uniform<T>(name);
    }

    // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
}
#endif