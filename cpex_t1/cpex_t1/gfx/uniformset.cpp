/**
 * gfx::uniformset - Shader uniform collection for material uniform definitions & overrides
 * ZIK@MMXXVI
 */

#include <gfx/uniformset.hpp>

using namespace gfx;

void UniformSet::update_texture_slots() {
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

std::shared_ptr<Uniform>& UniformSet::operator[](int idx) {
    return uniforms[idx];
}

size_t UniformSet::size() {
    return uniforms.size();
}