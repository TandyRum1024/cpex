/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <bit>

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

GLenum gfx::zmd2mdl::find_gl_prim_by_prim_type(const zmd2::PRIM_TYPE &type) {
    if (auto found = ZMD2_GL_PRIM_BY_PRIM_TBL.find(type); found != ZMD2_GL_PRIM_BY_PRIM_TBL.end()) {
        return found->second;
    }
    return 0;
}

std::shared_ptr<gfx::VertFormat> gfx::zmd2mdl::find_vert_format_by_model_type(const zmd2::MODEL_TYPE &type) {
    if (auto found = ZMD2_FORMAT_BY_MODEL_TBL.find(type); found != ZMD2_FORMAT_BY_MODEL_TBL.end()) {
        return found->second;
    }
    return 0;
}

Bbox::Bbox():
    min(0),
    max(0) {}

Bbox::Bbox(zmd2::BboxData *src):
    min(glm::make_vec3(src->min)),
    max(glm::make_vec3(src->max)) {}

void Bbox::merge_from(Bbox &other) {
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

Part::Part(zmd2::PartData *src):
    id(src->id),
    type(src->type),
    parentIdx(src->parentIdx),
    childrenIndices(src->childrenIndices) {}

Part::Part(std::string id, zmd2::PART_TYPE type):
    id(id),
    type(type),
    parentIdx(-1) {}

void Part::update_refs(Model *mdl) {
    auto allParts = mdl->all_parts();

    parent = allParts[parentIdx];
    children.resize(childrenIndices.size());
    for (auto i=0; i<childrenIndices.size(); i++) {
        children[i] = allParts[childrenIndices[i]];
    }
}

PartPoint::PartPoint(zmd2::PartPointData *src):
    Part(src),
    tfLocal(glm::make_vec3(src->tfLocal.pos), glm::make_vec4(src->tfLocal.rotQuaternion), glm::make_vec3(src->tfLocal.scale)),
    bounds(&src->bounds) {}

PartPoint::PartPoint(std::string id):
    Part(id, zmd2::PART_TYPE_POINT) {}

PartModel::PartModel(zmd2::PartModelData *src):
    Part(src),
    tfLocal(glm::make_vec3(src->tfLocal.pos), glm::make_vec4(src->tfLocal.rotQuaternion), glm::make_vec3(src->tfLocal.scale)),
    bounds(&src->bounds),
    modelType(src->modelType),
    modelPrim(src->modelPrim),
    modelPrimGl(find_gl_prim_by_prim_type(src->modelPrim)),
    modelVertFormat(find_vert_format_by_model_type(src->modelType)),
    morphIndices(src->morphIndices),
    materialIndices(src->materialIndices) {
        // Handle mesh data...
        matMeshes.reserve(src->matMeshes.size());
        for (auto&& srcMatMesh: src->matMeshes) {
            auto mesh = std::make_shared<gfx::Vb>();
            auto matMesh = std::make_shared<MaterialAndMeshPair>(MaterialAndMeshPair { .materialIdx = srcMatMesh->materialIdx, .material = nullptr, .mesh = mesh });

            mesh->set_format(modelVertFormat);
            modelVertFormat->set_buffer_from_bytes(srcMatMesh->meshData, &(*mesh), srcMatMesh->verticesNum);
            mesh->build();

            matMeshes.push_back(matMesh);
        }
    }

void PartModel::update_refs(Model *mdl) {
    Part::update_refs(mdl);

    auto allMorphs = mdl->all_morph_names();
    auto allMaterials = mdl->all_materials();

    morphNames.resize(morphIndices.size());
    materials.resize(materialIndices.size());
    for (auto i=0; i<morphIndices.size(); i++) {
        morphNames[i] = allMorphs[morphIndices[i]];
    }
    for (auto i=0; i<materialIndices.size(); i++) {
        materials[i] = allMaterials[materialIndices[i]];
    }
}

PartModel::PartModel(std::string id):
    Part(id, zmd2::PART_TYPE_MODEL),
    modelType(zmd2::MODEL_TYPE_MESH),
    modelPrim(zmd2::PRIM_TYPE_TRIANGLE_LIST),
    modelPrimGl(find_gl_prim_by_prim_type(modelPrim)) {}

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

Bone::Bone(zmd2::BoneData *src):
    id(src->id),
    parentIdx(src->parentIdx),
    childrenIndices(src->childrenIndices),
    length(length),
    tfLocal(glm::make_vec3(src->tfLocal.pos), glm::make_vec4(src->tfLocal.rotQuaternion), glm::make_vec3(src->tfLocal.scale)) {}

void Bone::update_refs(Model *mdl) {
    auto allBones = mdl->all_bones();

    parent = allBones[parentIdx];
    children.resize(childrenIndices.size());
    for (auto i=0; i<childrenIndices.size(); i++) {
        children[i] = allBones[childrenIndices[i]];
    }
}

Model::Model(zmd2::Model *src):
    id(src->id),
    bounds(&src->bounds),
    materialIds(src->materialNames),
    extraBytes(src->extraBytes), // hopefully this will be optimized by compiler into bulk copy...
    morphNames(src->morphNames) {
        bones.resize(src->bones.size());
        parts.resize(src->parts.size());

        // Convert bones
        std::transform(src->bones.begin(), src->bones.end(), bones.begin(), [](std::shared_ptr<zmd2::BoneData> bone){
            return std::make_shared<Bone>(&(*bone));
        });
        // Convert parts
        std::transform(src->parts.begin(), src->parts.end(), parts.begin(), [](std::shared_ptr<zmd2::PartData> part){
            std::shared_ptr<Part> conv;

            if (auto partPoint = std::static_pointer_cast<zmd2::PartPointData>(part)) {
                conv = std::make_shared<PartPoint>(&(*partPoint));
            }
            else if (auto partModel = std::static_pointer_cast<zmd2::PartModelData>(part)) {
                conv = std::make_shared<PartModel>(&(*partModel));
            }

            assert(conv);
            return conv;
        });

        // Load metadata
        auto meta = src->metadata;

        // (materials)
        if (auto embeddedMaterials = meta.find("embedded_materials"); embeddedMaterials != meta.end()) {
            for (auto matJson: *embeddedMaterials) {
                auto mat = matJson.get<MetaEmbeddedMaterial>();

                zcl::logger("ZMD2")->info("MODEL {}: EMBEDDED MAT {}", id, mat.name);
                zcl::logger("ZMD2")->info("{}", matJson.dump(1));
            }
        }

        // Call these later...
        /*
        // Process bones
        std::for_each(bones.begin(), bones.end(), [this](std::shared_ptr<Bone> bone){
            bone->update_refs(this);
        });
        // Process parts
        std::for_each(parts.begin(), parts.end(), [this](std::shared_ptr<Part> part){
            part->update_refs(this);
        });
        */
    }

Model::Model(std::string id):
    id(id) {}

void Model::load_embedded_materials(const zen::AssetManager &manager) {
    throw std::runtime_error("TODO load_embedded_materials");
}

void Model::load_embedded_textures(const zen::AssetManager &manager) {
    throw std::runtime_error("TODO load_embedded_textures");
}

void Model::link_material(const std::string materialId, const std::shared_ptr<gfx::Material> &material) {
    throw std::runtime_error("TODO link_material");
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

std::span<const std::shared_ptr<Bone>> Model::all_bones() const {
    return bones;
}

std::span<const std::shared_ptr<Part>> Model::all_parts() const {
    return parts;
}

std::span<const std::string> Model::all_morph_names() const {
    return morphNames;
}

std::span<const std::shared_ptr<gfx::Material>> Model::all_materials() const {
    return materials;
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


void gfx::zmd2mdl::from_json(const json &src, MaterialParams &val) {
    // src.at("name").get_to(val.);
}
void gfx::zmd2mdl::from_json(const json &src, MaterialUniform &val) {
    src.at("name").get_to(val.name);
}
void gfx::zmd2mdl::from_json(const json &src, MetaEmbeddedMaterial &val) {
    src.at("name").get_to(val.name);
    if (auto j = src.find("baseTexture"); j != src.end() && j.value().is_string()) {
        j.value().get_to(val.baseTextureId);
    }
    if (auto j = src.find("textures"); j != src.end() && j.value().is_array()) {
        j.value().get_to(val.textureIds);
    }
    if (auto j = src.find("shader"); j != src.end() && j.value().is_string()) {
        j.value().get_to(val.shaderId);
    }
    if (auto j = src.find("params"); j != src.end() && j.value().is_object()) {
        j.value().get_to(val.params);
    }
    if (auto j = src.find("params"); j != src.end() && j.value().is_array()) {
        j.value().get_to(val.uniforms);
    }
}