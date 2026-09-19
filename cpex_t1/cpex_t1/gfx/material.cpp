/**
 * gfx::material - Shader & uniform etc. abstraction
 * ZIK@MMXXVI
 */

#include <gfx/material.hpp>

using namespace gfx;

Material::Material(const std::string &id):
    Material(id, std::weak_ptr<Material>()) {}

Material::Material(const std::string &id, const std::weak_ptr<Material> &parent):
    id(id),
    parent(parent),
    isMergeRequired(true) {}

const std::vector<std::shared_ptr<Uniform>>& Material::get_uniforms() {
    process_merge();
    return mergedUniforms.get_uniforms();
}

bool Material::get_merge_required() const {
    return isMergeRequired;
}

inline void Material::process_merge() {
    if (auto parentMat = parent.lock()) {
        if (parentMat->get_merge_required()) {
            zcl::logger("GFX")->trace("{}: DETECTED PARENT {} MERGE REQUIRED...", id, parentMat->id);
            parentMat->process_merge();
        }
    }

    if (isMergeRequired) {
        zcl::logger("GFX")->trace("{}: MERGE!", id);
        if (shd) {
            mergedShd = shd;
        }
        else if (auto parentMat = parent.lock()) {
            mergedShd = parentMat->mergedShd;
        }
        else {
            mergedShd = nullptr;
        }

        mergedUniforms.clear();
        uniformLocations.clear();
        add_uniforms_to(mergedUniforms);

        for (auto&& uniform: mergedUniforms) {
            GLint location = 0;
            if (mergedShd) {
                location = mergedShd->get_uniform_location(uniform->get_name());
                // zcl::logger("GFX")->info("{}: FROM SHADER `{}`, UNIFORM `{}`: \t {}", id, shdMerged->get_name(), uniform->get_name(), location);
            }
            uniformLocations.push_back(location);
        }

        isMergeRequired = false;

        // Mark children for updates aswell
        for (auto&& child: children) {
            child->isMergeRequired = true;
        }
    }
}

void Material::add_uniforms_to(UniformSet &outUniforms) {
    if (auto parentMat = parent.lock()) {
        // parent->add_uniforms_to(outUniforms);
        outUniforms.add_uniforms_from(parentMat->mergedUniforms);
    }

    outUniforms.add_uniforms_from(uniforms);
}

void Material::set_shader(std::shared_ptr<Shader> shd) {
    this->shd = std::shared_ptr<Shader>(shd);

    // Update uniformsets to adapt newly set shader
    if (shd) {
        isMergeRequired = true;
    }
}

void Material::add_child(const std::shared_ptr<Material> &child) {
    children.push_back(child);
}

void Material::apply_material() {
    process_merge();

    if (mergedShd) {
        mergedShd->apply_shader();
    }
    
    // Apply all uniforms
    auto& allUniforms = mergedUniforms.get_uniforms();
    for (size_t i = 0; i < allUniforms.size(); i++) {
        auto& uniform = allUniforms[i];
        auto& location = uniformLocations[i];
        
        uniform->apply_uniform(location);
    }
}

std::shared_ptr<Material> gfx::material_make_inherited(std::shared_ptr<Material> parent, const std::string &childId) {
    auto child = std::make_shared<Material>(Material(childId, parent));
    parent->add_child(child);
    return child;
}