/**
 * gfx::zmd2::model - `.zmd2` model loading & wrapper for this projects graphics library.
 * ZIK@MMXXVI
 */

#include <variant>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <bit>

#include <gfx/zmd2/model.hpp>
#include <gfx/zmd2/utils.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>
#include <zcl/zlib.hpp>
#include <zmd2/types.hpp>
#include <zmd2/helper.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
// JSON
#include <nlohmann/json.hpp>
// fmt
#include <fmt/ranges.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx::zmd2;
using json = nlohmann::json;

// (helper struct for using `std::visitor`)
template<class... Ts>
struct overloads: Ts... {
    using Ts::operator()...;
};

// Constants definitions
const extern std::shared_ptr<gfx::VertFormat> gfx::zmd2::ZMD2_VERT_FORMAT_MESH = std::make_shared<gfx::VertFormat>(
        gfx::VertFormat({
            gfx::VertAttribute(0, 3, GL_FLOAT, sizeof(float)), // POS
            gfx::VertAttribute(1, 2, GL_FLOAT, sizeof(float)), // UV
            gfx::VertAttribute(2, 3, GL_FLOAT, sizeof(float)), // NORMAL
            gfx::VertAttribute(3, 4, GL_UNSIGNED_BYTE, sizeof(uint8_t)), // COL
        })
    );

const extern std::shared_ptr<gfx::VertFormat> gfx::zmd2::ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED = std::make_shared<gfx::VertFormat>(
        gfx::VertFormat({
            gfx::VertAttribute(0, 3, GL_FLOAT, sizeof(float)), // POS
            gfx::VertAttribute(1, 2, GL_FLOAT, sizeof(float)), // UV
            gfx::VertAttribute(2, 3, GL_FLOAT, sizeof(float)), // NORMAL
            gfx::VertAttribute(3, 4, GL_UNSIGNED_BYTE, sizeof(uint8_t)), // COL

            gfx::VertAttribute(4, 1, GL_FLOAT, sizeof(float)), // PART IDX
            gfx::VertAttribute(5, 4, GL_FLOAT, sizeof(float)), // BONE INDICES
            gfx::VertAttribute(6, 4, GL_FLOAT, sizeof(float)), // BONE WEIGHTS

            gfx::VertAttribute(7, 3, GL_FLOAT, sizeof(float)), // MORPH 1 POS OFF
            gfx::VertAttribute(8, 3, GL_FLOAT, sizeof(float)), // MORPH 1 NORMAL OFF
            gfx::VertAttribute(9, 3, GL_FLOAT, sizeof(float)), // MORPH 2 POS OFF
            gfx::VertAttribute(10, 3, GL_FLOAT, sizeof(float)), // MORPH 2 NORMAL OFF
        })
    );

Bbox::Bbox():
    min(0),
    max(0) {}

Bbox::Bbox(::zmd2::BboxData *src):
    min(glm::make_vec3(src->min)),
    max(glm::make_vec3(src->max)) {}

void Bbox::merge_from(Bbox &other) {
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

Part::Part(::zmd2::PartData *src):
    id(src->id),
    type(src->type),
    parentIdx(src->parentIdx),
    childrenIndices(src->childrenIndices) {}

Part::Part(std::string id, ::zmd2::PART_TYPE type):
    id(id),
    type(type),
    parentIdx(-1) {}

void Part::update_refs(Model *mdl) {
    auto allParts = mdl->all_parts();

    parent = (parentIdx != -1) ? allParts[parentIdx] : nullptr;
    children.resize(childrenIndices.size());
    for (auto i=0; i<childrenIndices.size(); i++) {
        auto childIdx = childrenIndices[i];

        children[i] = (childIdx != -1) ? allParts[childIdx] : nullptr;
    }
}

PartPoint::PartPoint(::zmd2::PartPointData *src):
    Part(src),
    tfLocal(glm::make_vec3(src->tfLocal.pos), glm::make_vec4(src->tfLocal.rotQuaternion), glm::make_vec3(src->tfLocal.scale)),
    bounds(&src->bounds) {}

PartPoint::PartPoint(std::string id):
    Part(id, ::zmd2::PART_TYPE_POINT) {}

PartModel::PartModel(::zmd2::PartModelData *src):
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

    morphNames.resize(morphIndices.size());
    for (auto i=0; i<morphIndices.size(); i++) {
        morphNames[i] = allMorphs[morphIndices[i]];
    }
}

void PartModel::link_materials(const std::vector<std::shared_ptr<Material>> &modelMaterials) {
    materials.resize(materialIndices.size());
    for (auto i=0; i<materialIndices.size(); i++) {
        materials[i] = modelMaterials[materialIndices[i]];
    }

    for (auto&& matMesh: matMeshes) {
        assert((matMesh->materialIdx >= 0) && (matMesh->materialIdx < modelMaterials.size()) && "Material index out of range!");
        if (!matMesh->material) {
            matMesh->material = modelMaterials[matMesh->materialIdx];
        }
    }
}

PartModel::PartModel(std::string id):
    Part(id, ::zmd2::PART_TYPE_MODEL),
    modelType(::zmd2::MODEL_TYPE_MESH),
    modelPrim(::zmd2::PRIM_TYPE_TRIANGLE_LIST),
    modelPrimGl(find_gl_prim_by_prim_type(modelPrim)) {}

std::shared_ptr<MaterialAndMeshPair> PartModel::reserve_matmesh(const std::string &materialId, std::shared_ptr<gfx::Material> &material) {
    auto foundMatMesh = matMeshesByMaterialId.find(materialId);

    if (foundMatMesh != matMeshesByMaterialId.end()) {
        return foundMatMesh->second;
    }

    ZMD2_DEBUG_TRACE_FUNC(warn, "PART {} MESHGROUP RESERVING => {}", id, materialId);

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

Bone::Bone(::zmd2::BoneData *src):
    id(src->id),
    parentIdx(src->parentIdx),
    childrenIndices(src->childrenIndices),
    length(length),
    tfLocal(glm::make_vec3(src->tfLocal.pos), glm::make_vec4(src->tfLocal.rotQuaternion), glm::make_vec3(src->tfLocal.scale)) {}

void Bone::update_refs(Model *mdl) {
    auto allBones = mdl->all_bones();

    parent = (parentIdx != -1) ? allBones[parentIdx] : nullptr;
    children.resize(childrenIndices.size());
    for (auto i=0; i<childrenIndices.size(); i++) {
        auto childIdx = childrenIndices[i];

        children[i] = (childIdx != -1) ? allBones[childIdx] : nullptr;
    }
}

gfx::Material MetaEmbeddedMaterial::to_material() const {
    auto newMatName = name;
    auto newMat = gfx::Material(newMatName);

    // Shader
    // newMat.set_shader(manager.load_shader(shader, false));

    // Base texture (for now, use `uAlbedo` uniform)

    // Uniforms 
    for (auto&& uniformInfo: uniforms) {
        std::visit(
            overloads {
                [&newMat, &newMatName](MetaUniformSampler val) {
                    newMat.add_uniforms(gfx::UniformSampler(
                            val.name, std::make_shared<gfx::Texture>(newMatName), (val.samplerTexfilter ? GL_LINEAR : GL_NEAREST), (val.samplerTexrepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE)
                        )
                    );
                },
                [&newMat](MetaUniformCol val) {
                    newMat.add_uniforms(gfx::UniformVec4(val.name, glm::make_vec4(val.value.data())));
                },
                [&newMat](MetaUniformFloat val) {
                    newMat.add_uniforms(gfx::UniformFloat(val.name, val.value));
                },
                [&newMat](MetaUniformInt val) {
                    newMat.add_uniforms(gfx::UniformInt(val.name, val.value));
                },
                [&newMat](MetaUniformVec val) {
                    switch (val.value.size()) {
                        case 2:
                            newMat.add_uniforms(gfx::UniformVec2(val.name, glm::make_vec2(val.value.data())));
                            break;
                        case 3:
                            newMat.add_uniforms(gfx::UniformVec3(val.name, glm::make_vec3(val.value.data())));
                            break;
                        case 4:
                            newMat.add_uniforms(gfx::UniformVec4(val.name, glm::make_vec4(val.value.data())));
                            break;
                        default:
                            throw std::runtime_error(fmt::format("Vector uniform length of {} are not supported!", val.value.size()));
                            break;
                    }
                },
                [&newMat](MetaUniformIvec val) {
                    switch (val.value.size()) {
                        case 2:
                            newMat.add_uniforms(gfx::UniformIvec2(val.name, glm::make_vec2(val.value.data())));
                            break;
                        case 3:
                            newMat.add_uniforms(gfx::UniformIvec3(val.name, glm::make_vec3(val.value.data())));
                            break;
                        case 4:
                            newMat.add_uniforms(gfx::UniformIvec4(val.name, glm::make_vec4(val.value.data())));
                            break;
                        default:
                            throw std::runtime_error(fmt::format("Vector uniform length of {} are not supported!", val.value.size()));
                            break;
                    }
                },
                [&newMat](MetaUniformMat4 val) {
                    newMat.add_uniforms(gfx::UniformMat4(val.name, val.value));
                }
            },
            uniformInfo
        );
    }

    return newMat;
}

gfx::Texture MetaEmbeddedTexture::to_texture(std::span<const uint8_t> bytes) const {
    auto newTexName = name;
    auto newTex = gfx::Texture(newTexName, GL_TEXTURE_2D);
    auto blobSpan = bytes.subspan(blobOffset, blobSize);
    auto internalFmt = find_gl_tex_base_format_by_tex_format(format);
    auto dataFmt = find_gl_tex_data_format_by_tex_format(format);

    // ZMD2_DEBUG_TRACE_FUNC(info, "TEX {} ({}x{})", name, width, height);
    newTex.set_format(internalFmt);
    newTex.load_from_buffer_2d(blobSpan.data(), width, height, dataFmt, GL_UNSIGNED_BYTE);

    return newTex;
}

Model::Model(::zmd2::Model *src):
    id(src->id),
    bounds(&src->bounds),
    materialIds(src->materialNames),
    extraBytes(src->extraBytes), // hopefully this will be optimized by compiler into bulk copy...
    morphNames(src->morphNames) {
        bones.resize(src->bones.size());
        parts.resize(src->parts.size());

        // Convert bones
        ZMD2_DEBUG_TRACE_FUNC(info, "MODEL {}: {} BONES", id, src->bones.size());
        std::transform(src->bones.begin(), src->bones.end(), bones.begin(), [](std::shared_ptr<::zmd2::BoneData> bone){
            ZMD2_DEBUG_TRACE_FUNC(info, "\t`{}` (PARENT: {}, {} CHILDREN: [{}])", bone->id, (bone->parentIdx == -1) ? "<NONE>" : fmt::format("{}", bone->parentIdx), bone->childrenIndices.size(), fmt::join(bone->childrenIndices, ", "));
            return std::make_shared<Bone>(&(*bone));
        });
        // Convert parts
        ZMD2_DEBUG_TRACE_FUNC(info, "MODEL {}: {} PARTS", id, src->parts.size());
        std::transform(src->parts.begin(), src->parts.end(), parts.begin(), [](std::shared_ptr<::zmd2::PartData> part){
            std::shared_ptr<Part> conv;

            switch (part->type) {
                case ::zmd2::PART_TYPE_MODEL:
                    if (auto partModel = std::static_pointer_cast<::zmd2::PartModelData>(part)) {
                        conv = std::make_shared<PartModel>(&(*partModel));
                        ZMD2_DEBUG_TRACE_FUNC(info, "\t>`{}` (TYPE: MODEL)", part->id);
                    }
                    break;
                case ::zmd2::PART_TYPE_POINT:
                    if (auto partPoint = std::static_pointer_cast<::zmd2::PartPointData>(part)) {
                        conv = std::make_shared<PartPoint>(&(*partPoint));
                        ZMD2_DEBUG_TRACE_FUNC(info, "\t>`{}` (TYPE: POINT)", part->id);
                    }
                    break;
                default:
                    ZMD2_DEBUG_TRACE_FUNC(error, "UNKNOWN PART TYPE: {}", fmt::underlying(part->type));
                    break;
            }
            ZMD2_DEBUG_TRACE_FUNC(info, "\t(PARENT: {}, {} CHILDREN: [{}])", (conv->parentIdx == -1) ? "<NONE>" : fmt::format("{}", conv->parentIdx), conv->childrenIndices.size(), fmt::join(conv->childrenIndices, ", "));

            assert(conv);
            return conv;
        });

        // Load metadata
        auto meta = src->metadata;

        // (embedded materials)
        ZMD2_DEBUG_TRACE_FUNC(info, "MODEL {}: EMBEDDED MAT", id);
        if (auto metaMaterialsFound = meta.find("embedded_materials"); metaMaterialsFound != meta.end()) {
            auto metaMaterials = *metaMaterialsFound;

            embeddedMaterials.reserve(metaMaterials.size());
            for (auto matJson: metaMaterials) {
                auto mat = matJson.get<MetaEmbeddedMaterial>();
                ZMD2_DEBUG_TRACE_FUNC(info, "{}", mat);
                embeddedMaterials.push_back(mat);
            }
        }

        // (embedded textures)
        ZMD2_DEBUG_TRACE_FUNC(info, "MODEL {}: EMBEDDED TEX", id);
        if (auto metaTexturesFound = meta.find("textures"); metaTexturesFound != meta.end()) {
            auto metaTextures = *metaTexturesFound;

            embeddedTextures.reserve(metaTextures.size());
            for (auto texJson: metaTextures) {
                auto tex = texJson.get<MetaEmbeddedTexture>();
                ZMD2_DEBUG_TRACE_FUNC(info, "{}", tex);
                embeddedTextures.push_back(tex);
            }
        }

        // Prepare textures linking
        linkedMaterials.resize(materialIds.size());
    }

Model::Model(std::string id):
    id(id) {}

// void Model::load_and_find_embedded_assets(zen::AssetManager &manager, const std::shared_ptr<gfx::Material> &fallbackMaterial, const std::shared_ptr<gfx::Texture> &fallbackTexture) {
//     // Add embedded assets if not existing
//     for (auto&& texInfo: embeddedTextures) {
//         if (manager.get_texture(texInfo.name)) {
//             continue;
//         }

//         // ZMD2_DEBUG_TRACE_FUNC(error, "MODEL {}: EMBEDDED TEX {} ({}x{})", id, texInfo.name, texInfo.width, texInfo.height);
//         auto newTex = texInfo.to_texture(extraBytes);
//         auto newTexPath = newTex.get_id();
//         // ZMD2_DEBUG_TRACE_FUNC(error, "\t(EMBEDDED TEX {} FIN)", texInfo.name);
//         manager.add_texture(newTexPath, std::make_shared<gfx::Texture>(std::move(newTex)));
//     }

//     for (auto&& matInfo: embeddedMaterials) {
//         if (manager.get_material(matInfo.name)) {
//             continue;
//         }

//         auto newMat = matInfo.to_material(manager);

//         manager.add_material(newMat.get_id(), std::make_shared<gfx::Material>(std::move(newMat)));
//     }

//     // Link assets
//     materials.reserve(materialIds.size());
//     for (auto&& matId: materialIds) {
//         auto mat = manager.get_material(matId);

//         if (!mat) {
//             if (fallbackMaterial) {
//                 materials.push_back(fallbackMaterial);
//             }
//             else {
//                 materials.push_back(manager.get_material(ZMD2_ASSET_BASE_MATERIAL));
//             }
//             continue;
//         }

//         materials.push_back(mat);
//     }
// }

void Model::update_refs_and_link_assets(const std::shared_ptr<gfx::Material> &fallbackMaterial) {
    // Build list of materials
    auto materials = std::vector<std::shared_ptr<gfx::Material>>();

    materials.resize(linkedMaterials.size());
    std::transform(linkedMaterials.begin(), linkedMaterials.end(), materials.begin(), [&fallbackMaterial](std::shared_ptr<gfx::Material> mat) {
        return mat ? mat : fallbackMaterial;
    });

    // Process bones
    bonesById.clear();
    std::for_each(bones.begin(), bones.end(), [this](std::shared_ptr<Bone> bone){
        bonesById[bone->id] = bone;
        bone->update_refs(this);
    });
    
    // Process parts
    partsById.clear();
    std::for_each(parts.begin(), parts.end(), [this, &materials](std::shared_ptr<Part> part){
        partsById[part->id] = part;
        part->update_refs(this);

        switch (part->type) {
            case ::zmd2::PART_TYPE_MODEL:
                if (auto partModel = std::static_pointer_cast<PartModel>(part)) {
                    partModel->link_materials(materials);
                }
                break;
        }
    });
}

void Model::add_part(const std::shared_ptr<Part> &part) {
    parts.push_back(part);
    partsById[part->id] = part;
}

void Model::add_bone(const std::shared_ptr<Bone> &bone) {
    bones.push_back(bone);
    bonesById[bone->id] = bone;
}

void Model::link_material(const int idx, const std::shared_ptr<gfx::Material> &material) {
    linkedMaterials[idx] = material;
}

void Model::link_texture(const std::string &textureId, const std::shared_ptr<gfx::Texture> &texture) {
    linkedTexturesById[textureId] = texture;
}

std::shared_ptr<Bone> Model::find_bone_by_id(const std::string &id) const {
    auto res = bonesById.find(id);
    
    if (res == bonesById.end()) {
        return nullptr;
    }
    return (*res).second;
}

std::shared_ptr<gfx::Material> Model::find_linked_material_by_id(const std::string &id) const {
    for (auto&& mat: linkedMaterials) {
        if (mat->get_id() == id) {
            return mat;
        }
    }
    
    return nullptr;
}

std::shared_ptr<gfx::Texture> Model::find_linked_texture_by_id(const std::string &id) const {
    auto res = linkedTexturesById.find(id);
    
    if (res == linkedTexturesById.end()) {
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

std::span<const std::string> Model::all_material_ids() const {
    return materialIds;
}

std::span<const std::string> Model::all_morph_names() const {
    return morphNames;
}

std::span<const MetaEmbeddedMaterial> Model::all_embedded_materials() const {
    return embeddedMaterials;
}

std::span<const MetaEmbeddedTexture> Model::all_embedded_textures() const {
    return embeddedTextures;
}

std::span<const uint8_t> Model::get_extra_bytes() const {
    return extraBytes;
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
                        ZMD2_DEBUG_TRACE_FUNC(error, "MODEL {}: PART {} HAS NO MESH FOR MESHGROUP {}!", id, partModel->id, material->get_id());
                    }
                    else {
                        ZMD2_DEBUG_TRACE_FUNC(error, "MODEL {}: PART {} HAS NO MESH OR MATERIAL!", id, partModel->id);
                    }
                    continue;
                }
                if (!partModel->modelPrimGl) {
                    ZMD2_DEBUG_TRACE_FUNC(error, "MODEL {}: PART {} HAS INVALID PRIMITIVE TYPE {}!", id, partModel->id, static_cast<int>(partModel->modelPrim));
                }

                material->apply_material(texManager);
                mesh->submit(partModel->modelPrimGl, 0);
            }
        }
    }
}

/** "Safe get" with default value, accounts for (top level) value being `null`. */
template <typename T>
void json_get_to(const json &src, const char key[], T &dst, T fallback) {
    if (auto j = src.find(key); j != src.end() && !(*j).is_null()) {
        (*j).get_to(dst);
    }
    else {
        dst = fallback;
    }
}

template <typename T>
void json_get_to(const json &src, const char key[], T &dst) {
    if (auto j = src.find(key); j != src.end() && !(*j).is_null()) {
        (*j).get_to(dst);
    }
}

void gfx::zmd2::from_json(const json &src, MetaUniformAny &val) {
    std::string uniformType = src.at("type").get<std::string>();

    // TODO: this is idiotic. use a table to look up lambdas that reads appropriate types maybe
    // For now use "dumb" pseudo switch and pray that compilers will recognize my woes
    if (uniformType == "COL") {
        val = src.get<MetaUniformCol>();
    }
    else if (uniformType == "FLOAT") {
        val = src.get<MetaUniformFloat>();
    }
    else if (uniformType == "INT") {
        val = src.get<MetaUniformInt>();
    }
    else if (uniformType == "SAMPLER") {
        val = src.get<MetaUniformSampler>();
    }
    else if (uniformType == "VEC") {
        val = src.get<MetaUniformVec>();
    }
    else if (uniformType == "IVEC") {
        val = src.get<MetaUniformIvec>();
    }
    else if (uniformType == "MAT4") {
        val = src.get<MetaUniformMat4>();
    }
    else {
        throw std::runtime_error(fmt::format("Invalid uniform type `{}`!", uniformType));
    }
}

void gfx::zmd2::from_json(const json &src, MetaMaterialParams &val) {
    // Take care of nullable types
    __ZMD2_TO_JSON_V(src, val, cull);
    __ZMD2_TO_JSON_V(src, val, zwrite);
    __ZMD2_TO_JSON_V(src, val, ztest);
    __ZMD2_TO_JSON_V(src, val, zfunc);
    __ZMD2_TO_JSON_V(src, val, alphatest);
    __ZMD2_TO_JSON_V(src, val, alphatestRef);
    __ZMD2_TO_JSON_V(src, val, baseTexfilter);
    __ZMD2_TO_JSON_V(src, val, baseTexrepeat);
    __ZMD2_TO_JSON_NULL(src, val, chainMaterialNext);
    __ZMD2_TO_JSON_NULL(src, val, shadowPassMaterial);
    __ZMD2_TO_JSON_V(src, val, shadowPassMaterialCopy);
    __ZMD2_TO_JSON_NULL(src, val, shadowPassMaterialCopyShader);
    __ZMD2_TO_JSON_V(src, val, transparent);
    __ZMD2_TO_JSON_V(src, val, blendmodeSrc);
    __ZMD2_TO_JSON_V(src, val, blendmodeDst);
    __ZMD2_TO_JSON_V(src, val, blendmodeEq);
}

void gfx::zmd2::from_json(const json &src, MetaEmbeddedMaterial &val) {
    // Take care of nullable types
    __ZMD2_TO_JSON_V(src, val, name);
    __ZMD2_TO_JSON_V(src, val, shader);
    __ZMD2_TO_JSON_NULL(src, val, baseTexture);
    __ZMD2_TO_JSON_V(src, val, textures);
    __ZMD2_TO_JSON_V(src, val, params);
    __ZMD2_TO_JSON_V(src, val, uniforms);
}

std::string MetaMaterialParams::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniform::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformSampler::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformCol::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformFloat::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformInt::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformVec::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformIvec::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaUniformMat4::to_string() const {
    json j = *this;
    return j.dump();
}
std::string MetaEmbeddedMaterial::to_string() const {
    std::stringstream stream;

    stream << fmt::format("MATERIAL `{}` (shader: `{}`, baseTexture: `{}`)", name, shader, baseTexture) << std::endl;
    for (auto&& tex: textures) {
        stream << fmt::format("\tLINKED TEX: `{}`", tex) << std::endl;
    }
    stream << fmt::format("\tPARAMS: {}", params) << std::endl;
    for (auto&& uniform: uniforms) {
        std::string res = std::visit(
            overloads {
                [](MetaUniformSampler val){ return val.to_string(); },
                [](MetaUniformCol val){ return val.to_string(); },
                [](MetaUniformFloat val){ return val.to_string(); },
                [](MetaUniformInt val){ return val.to_string(); },
                [](MetaUniformVec val){ return val.to_string(); },
                [](MetaUniformIvec val){ return val.to_string(); },
                [](MetaUniformMat4 val){ return val.to_string(); },
                [](MetaUniform val){ return fmt::format("<`{}`: Unknown uniform type `{}`!!>", val.name, val.type); }
            },
            uniform
        );

        stream << fmt::format("\tUNIFORM: {}", res) << std::endl;
    }
    
    auto str = stream.str();
    // Trim newline
    // https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
    str.erase(str.find_last_not_of("\r\n") + 1);
    return str;
}
std::string MetaEmbeddedTexture::to_string() const {
    std::stringstream stream;
    json fmt = format;

    stream << fmt::format("\tTEXTURE `{}` ({}x{}, embedded: {}, format: {})", name, width, height, embedded, fmt.dump()) << std::endl;

    auto str = stream.str();
    // Trim newline
    // https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
    str.erase(str.find_last_not_of("\r\n") + 1);
    return str;
}

std::string gfx::zmd2::format_as(MetaEmbeddedTexture val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaEmbeddedMaterial val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaMaterialParams val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformSampler val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformCol val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformFloat val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformInt val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformVec val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformIvec val) { return val.to_string(); };
std::string gfx::zmd2::format_as(MetaUniformMat4 val) { return val.to_string(); };