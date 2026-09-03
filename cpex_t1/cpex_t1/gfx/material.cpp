/**
 * gfx::material - Shader & uniform etc. abstraction
 * ZIK@MMXXVI
 */

#include <gfx/material.hpp>
#include <gfx/uniform.hpp>

using namespace gfx;

void Material::update_texture_slots() {
    texSlotIdx = 0;

    // Apply all uniforms
    for (size_t i = 0; i < uniforms.size(); i++) {
        auto uniform = uniforms[i];
        
        if (auto sampler = std::dynamic_pointer_cast<UniformSampler2D>(uniform)) {
            sampler->set_tex_slot(texSlotIdx);
            texSlotIdx++;
        }
    }
}

void Material::set_shader(std::shared_ptr<Shader> shd) {
    this->shd = std::shared_ptr<Shader>(shd);

    // Update uniformsets to adapt newly set shader
    if (this->shd) {
        for (size_t i = 0; i < uniforms.size(); i++) {
            auto uniform = uniforms[i];
            auto location = this->shd->get_uniform_location(uniform->get_name());

            uniformLocations[i] = location;
        }
        update_texture_slots();
    }
}

void Material::apply_material() {
    if (shd) {
        shd->apply_shader();

        // Apply all uniforms
        for (size_t i = 0; i < uniforms.size(); i++) {
            auto uniform = uniforms[i];
            auto location = uniformLocations[i];
            
            uniform->apply_uniform(location);
        }
    }
}