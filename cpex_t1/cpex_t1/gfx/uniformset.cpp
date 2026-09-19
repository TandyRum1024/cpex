/**
 * gfx::uniformset - Shader uniform collection for material uniform definitions & overrides
 * ZIK@MMXXVI
 */

#include <gfx/uniformset.hpp>

using namespace gfx;

// delegate to internal uniform vectors

const std::vector<std::shared_ptr<Uniform>>& UniformSet::get_uniforms() const {
    return uniforms;
}

std::vector<std::shared_ptr<Uniform>>::const_iterator UniformSet::begin() {
    return uniforms.cbegin();
}

std::vector<std::shared_ptr<Uniform>>::const_iterator UniformSet::end() {
    return uniforms.cend();
}

void UniformSet::add_uniform(const std::shared_ptr<Uniform> &uniformPtr) {
    auto name = uniformPtr->get_name();
    
    // Override if needed
    if (uniformsByName.contains(name)) {
        std::pair<size_t, std::shared_ptr<Uniform>> entry = uniformsByName[name];
        entry.second = uniformPtr;
    }
    else {
        size_t idx = uniforms.size();
        uniforms.push_back(uniformPtr);
        uniformsByName[name] = std::pair(idx, uniformPtr);
    }
}

void UniformSet::add_uniforms_from(const UniformSet &other) {
    for (auto&& uniform: other.uniforms) {
        add_uniform(uniform);
    }
}

size_t UniformSet::size() const {
    return uniforms.size();
}

void UniformSet::clear() {
    uniforms.clear();
    uniformsByName.clear();
}