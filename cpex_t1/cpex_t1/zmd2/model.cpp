/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <stdexcept>
#include <sstream>

#include <zmd2/model.hpp>
#include <zmd2/types.hpp>
#include <zmd2/helper.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>
#include <zcl/zlib.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
// JSON
#include <nlohmann/json.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;
using json = nlohmann::json;

Zmd2Model::Zmd2Model(std::string id):
    id(id) {}

std::shared_ptr<MaterialAndMeshPair> Zmd2Model::reserve_meshgroup(const std::string &materialId) {
    return nullptr;
    
    // if (meshGroupsByMaterialId.contains(materialId)) {
    //     return meshGroupsByMaterialId.at(materialId);
    // }

    // zcl::logger("ZMD2")->warn("MODEL {} MESHGROUP RESERVING => {}", id, materialId);

    // // Make a new meshgroup entry
    // // auto mesh = std::make_shared<gfx::Vb>();
    // auto meshGroup = std::make_shared<LoadMeshGroup>(LoadMeshGroup { .material = nullptr, .mesh = nullptr });

    // meshGroups.push_back(meshGroup);
    // // materials.push_back(material);
    // // meshes.push_back(mesh);

    // meshGroupsByMaterialId[materialId] = meshGroup;

    // return meshGroup;
}

std::shared_ptr<MaterialAndMeshPair> Zmd2Model::find_meshgroup_by_material_id(const std::string &materialId) const {
    // auto res = meshGroupsByMaterialId.find(materialId);
    
    // if (res == meshGroupsByMaterialId.end()) {
    //     return nullptr;
    // }
    // return (*res).second;
    return nullptr;
}

void Zmd2Model::submit(gfx::TextureManager &texManager) const {
    // zcl::logger("ZMD2")->warn("SUBMIT MODEL {}...", id);

    return;

    // FIXME
    // for (auto &&meshGroup: meshGroups) {
    //     auto material = meshGroup->material;
    //     auto mesh = meshGroup->mesh;

    //     if (!material || !mesh) {
    //         if (material) {
    //             zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH FOR MESHGROUP {}!", id, material->get_id());
    //         }
    //         else {
    //             zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH OR MATERIAL!", id);
    //         }
    //         continue;
    //     }

    //     // zcl::logger("ZMD2")->warn("MODEL {} SUBMIT! ({}, {} verts)", id, material->get_id(), mesh->get_indices_num());
    //     material->apply_material(texManager);
    //     mesh->submit(GL_TRIANGLES, 0);
    // }
}

