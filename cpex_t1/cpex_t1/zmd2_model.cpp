/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <stdexcept>
#include <sstream>

#include <zmd2_model.hpp>

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
#include <glm/gtc/type_ptr.hpp>
// JSON
#include <nlohmann/json.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx::zmd2mdl;
using json = nlohmann::json;

GLenum gfx::zmd2mdl::find_gl_prim_by_prim(const zmd2::PRIM_TYPE &type) {
    if (auto found = ZMD2_GL_PRIM_BY_PRIM_TBL.find(type); found != ZMD2_GL_PRIM_BY_PRIM_TBL.end()) {
        return found->second;
    }
    return 0;
}

Bbox::Bbox():
    min(0),
    max(0) {}

Bbox::Bbox(zmd2::BboxData src):
    min(glm::make_vec3(src.min)),
    max(glm::make_vec3(src.max)) {}

void Bbox::merge_from(Bbox &other) {
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

Part::Part(zmd2::PartData src):
    id(src.id),
    type(src.type),
    parentIdx(src.parentIdx),
    childrenIndices(src.childrenIndices) {}

Part::Part(std::string id, zmd2::PART_TYPE type):
    id(id),
    type(type),
    parentIdx(-1) {}

PartPoint::PartPoint(zmd2::PartPointData src):
    Part(src),
    tfLocal(glm::make_vec3(src.tfLocal.pos), glm::make_vec4(src.tfLocal.rotQuaternion), glm::make_vec3(src.tfLocal.scale)),
    bounds(src.bounds) {}

PartPoint::PartPoint(std::string id):
    Part(id, zmd2::PART_TYPE_POINT) {}

PartModel::PartModel(zmd2::PartModelData src):
    Part(src),
    tfLocal(glm::make_vec3(src.tfLocal.pos), glm::make_vec4(src.tfLocal.rotQuaternion), glm::make_vec3(src.tfLocal.scale)),
    bounds(src.bounds),
    modelType(src.modelType),
    modelPrim(src.modelPrim),
    modelPrimGl(find_gl_prim_by_prim(src.modelPrim)) {}

PartModel::PartModel(std::string id):
    Part(id, zmd2::PART_TYPE_MODEL),
    modelType(zmd2::MODEL_TYPE_MESH),
    modelPrim(zmd2::PRIM_TYPE_TRIANGLE_LIST),
    modelPrimGl(find_gl_prim_by_prim(modelPrim)) {}

std::shared_ptr<MaterialAndMeshPair> PartModel::reserve_matmesh(const std::string &materialId, std::shared_ptr<gfx::Material> &material) {
    auto foundMatMesh = matMeshesByMaterialId.find(materialId);

    if (foundMatMesh != matMeshesByMaterialId.end()) {
        return foundMatMesh->second;
    }

    zcl::logger("ZMD2")->warn("PART {} MESHGROUP RESERVING => {}", id, materialId);

    // Make a new meshgroup entry
    auto matMesh = std::make_shared<MaterialAndMeshPair>(MaterialAndMeshPair { .materialIdx = 0, .material = material, .mesh = nullptr });

    matMeshes.push_back(matMesh);
    // materials.push_back(material);
    // meshes.push_back(mesh);
    materials.push_back(material);
    matMeshesByMaterialId[materialId] = matMesh;

    return matMesh;
}

std::shared_ptr<MaterialAndMeshPair> PartModel::find_matmesh_by_material_id(const std::string &materialId) const {
    auto res = matMeshesByMaterialId.find(materialId);
    
    if (res == matMeshesByMaterialId.end()) {
        return nullptr;
    }
    return (*res).second;
}

Bone::Bone(zmd2::BoneData src):
    id(src.id),
    parentIdx(src.parentIdx),
    childrenIndices(src.childrenIndices),
    length(length),
    tfLocal(glm::make_vec3(src.tfLocal.pos), glm::make_vec4(src.tfLocal.rotQuaternion), glm::make_vec3(src.tfLocal.scale)) {}

Model::Model(std::string id):
    id(id) {}

void Model::load_embedded_materials(const zen::AssetManager &manager) {

}

void Model::load_embedded_textures(const zen::AssetManager &manager) {

}

void Model::link_material(const std::string materialId, const std::shared_ptr<gfx::Material> &material) {

}

void Model::add_part(const std::shared_ptr<Part> &part) {
    parts.push_back(part);
    partsById[part->id] = part;
}

void Model::add_bone(const std::shared_ptr<Bone> &bone) {
    bones.push_back(bone);
    bonesById[bone->id] = bone;
}

std::shared_ptr<Bone> Model::find_bone_by_id(const std::string &id) const {
    auto res = bonesById.find(id);
    
    if (res == bonesById.end()) {
        return nullptr;
    }
    return (*res).second;
}

void Model::submit(gfx::TextureManager &texManager) const {
    for (auto&& part: parts) {
        // (Only draw mesh models for now)
        if (auto partModel = std::static_pointer_cast<PartModel>(part)) {
            for (auto&& matMesh: partModel->matMeshes) {
                auto material = matMesh->material;
                auto mesh = matMesh->mesh;

                if (!material || !mesh) {
                    if (material) {
                        zcl::logger("ZMD2")->error("MODEL {}: PART {} HAS NO MESH FOR MESHGROUP {}!", id, partModel->id, material->get_id());
                    }
                    else {
                        zcl::logger("ZMD2")->error("MODEL {}: PART {} HAS NO MESH OR MATERIAL!", id, partModel->id);
                    }
                    continue;
                }
                if (!partModel->modelPrimGl) {
                    zcl::logger("ZMD2")->error("MODEL {}: PART {} HAS INVALID PRIMITIVE TYPE {}!", id, partModel->id, static_cast<int>(partModel->modelPrim));
                }

                material->apply_material(texManager);
                mesh->submit(partModel->modelPrimGl, 0);
            }
        }
    }
}
