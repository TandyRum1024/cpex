/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <zmd2/model.hpp>

using namespace zmd2;

std::shared_ptr<MeshGroup> Model::reserve_meshgroup(const std::string &materialId) {
    if (meshGroupsByMaterialId.contains(materialId)) {
        return meshGroupsByMaterialId.at(materialId);
    }

    zcl::logger("ZMD2")->trace("MODEL {} MESHGROUP RESERVING => {}", id, materialId);

    // Make a new meshgroup entry
    // auto mesh = std::make_shared<gfx::Vb>();
    auto meshGroup = std::make_shared<MeshGroup>(MeshGroup { .material = nullptr, .mesh = nullptr });

    meshGroups.push_back(meshGroup);
    // materials.push_back(material);
    // meshes.push_back(mesh);

    meshGroupsByMaterialId[materialId] = meshGroup;

    return meshGroup;
}

std::shared_ptr<MeshGroup> Model::find_meshgroup_by_material_id(const std::string &materialId) const {
    auto res = meshGroupsByMaterialId.find(materialId);
    
    if (res == meshGroupsByMaterialId.end()) {
        return nullptr;
    }
    return (*res).second;
}

void Model::submit(gfx::TextureManager texManager) const {
    for (auto &&meshGroup: meshGroups) {
        auto material = meshGroup->material;
        auto mesh = meshGroup->mesh;

        if (!material || !mesh) {
            continue;
        }

        material->apply_material(texManager);
        mesh->submit(GL_TRIANGLES, 0);
    }
}