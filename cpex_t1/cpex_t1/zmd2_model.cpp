/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <variant>
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

// (helper struct for using `std::visitor`)
template<class... Ts>
struct overloads: Ts... {
    using Ts::operator()...;
};

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

GLenum gfx::zmd2mdl::find_gl_tex_base_format_by_tex_format(const ZMD2_TEX_FORMAT &type) {
    if (auto found = ZMD2_TEX_FORMAT_TO_GL_TEX_INTERNAL_FORMAT_TBL.find(type); found != ZMD2_TEX_FORMAT_TO_GL_TEX_INTERNAL_FORMAT_TBL.end()) {
        return found->second;
    }
    return GL_RGBA8;
}

GLenum gfx::zmd2mdl::find_gl_tex_data_format_by_tex_format(const ZMD2_TEX_FORMAT &type) {
    if (auto found = ZMD2_TEX_FORMAT_TO_GL_TEX_FORMAT_TBL.find(type); found != ZMD2_TEX_FORMAT_TO_GL_TEX_FORMAT_TBL.end()) {
        return found->second;
    }
    return GL_RGBA8;
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

gfx::Material MetaEmbeddedMaterial::to_material(zen::AssetManager &manager) const {
    auto newMatName = name;
    auto newMat = gfx::Material(newMatName);

    newMat.set_shader(manager.load_shader(shader, false));
    for (auto&& uniformInfo: uniforms) {
        std::visit(
            overloads {
                [&newMat, &manager](MetaUniformSampler val) {
                    newMat.add_uniforms(gfx::UniformSampler(
                            val.name, manager.load_texture(val.value), val.samplerTexfilter ? GL_LINEAR : GL_NEAREST, val.samplerTexrepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE
                        )
                    );
                },
                [&newMat, &manager](MetaUniformCol val) {
                    newMat.add_uniforms(gfx::UniformVec4(val.name, glm::make_vec4(val.value.data())));
                },
                [&newMat, &manager](MetaUniformFloat val) {
                    newMat.add_uniforms(gfx::UniformFloat(val.name, val.value));
                },
                [&newMat, &manager](MetaUniformInt val) {
                    newMat.add_uniforms(gfx::UniformInt(val.name, val.value));
                },
                [&newMat, &manager](MetaUniformVec val) {
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
                [&newMat, &manager](MetaUniformIvec val) {
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
                [&newMat, &manager](MetaUniformMat4 val) {
                    newMat.add_uniforms(gfx::UniformMat4(val.name, val.value));
                }
            },
            uniformInfo
        );
    }

    return newMat;
}

gfx::Texture MetaEmbeddedTexture::to_texture(const std::span<uint8_t> &bytes) const {
    auto newTexName = name;
    auto newTex = gfx::Texture(newTexName, GL_TEXTURE_2D);
    auto blobSpan = bytes.subspan(blobOffset, blobSize);
    auto internalFmt = find_gl_tex_base_format_by_tex_format(format);
    auto dataFmt = find_gl_tex_data_format_by_tex_format(format);

    // zcl::logger("ZMD2")->info("TEX {} ({}x{})", name, width, height);
    newTex.set_format(internalFmt);
    newTex.load_from_buffer_2d(blobSpan.data(), width, height, dataFmt, GL_UNSIGNED_BYTE);

    return newTex;
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

        // (embedded materials)
        // zcl::logger("ZMD2")->info("MODEL {}: EMBEDDED MAT", id);
        if (auto metaMaterialsFound = meta.find("embedded_materials"); metaMaterialsFound != meta.end()) {
            auto metaMaterials = *metaMaterialsFound;

            embeddedMaterials.reserve(metaMaterials.size());
            for (auto matJson: metaMaterials) {
                auto mat = matJson.get<MetaEmbeddedMaterial>();
                // zcl::logger("ZMD2")->info("{}", mat);
                embeddedMaterials.push_back(mat);
            }
        }

        // (embedded textures)
        // zcl::logger("ZMD2")->info("MODEL {}: EMBEDDED TEX", id);
        if (auto metaTexturesFound = meta.find("textures"); metaTexturesFound != meta.end()) {
            auto metaTextures = *metaTexturesFound;

            embeddedTextures.reserve(metaTextures.size());
            for (auto texJson: metaTextures) {
                auto tex = texJson.get<MetaEmbeddedTexture>();
                // zcl::logger("ZMD2")->info("{}", tex);
                embeddedTextures.push_back(tex);
            }
        }
    }

Model::Model(std::string id):
    id(id) {}

void Model::load_and_find_embedded_assets(zen::AssetManager &manager, const std::shared_ptr<gfx::Material> &fallbackMaterial, const std::shared_ptr<gfx::Texture> &fallbackTexture) {
    // Add embedded assets if not existing
    for (auto&& texInfo: embeddedTextures) {
        if (manager.get_texture(texInfo.name)) {
            continue;
        }

        // zcl::logger("ZMD2")->error("MODEL {}: EMBEDDED TEX {} ({}x{})", id, texInfo.name, texInfo.width, texInfo.height);
        auto newTex = texInfo.to_texture(extraBytes);
        auto newTexPath = newTex.get_id();
        // zcl::logger("ZMD2")->error("\t(EMBEDDED TEX {} FIN)", texInfo.name);
        manager.add_texture(newTexPath, std::make_shared<gfx::Texture>(std::move(newTex)));
    }

    for (auto&& matInfo: embeddedMaterials) {
        if (manager.get_material(matInfo.name)) {
            continue;
        }

        auto newMat = matInfo.to_material(manager);

        manager.add_material(newMat.get_id(), std::make_shared<gfx::Material>(std::move(newMat)));
    }

    // Link assets
    // materials.resize(materialIds.size());
    // for (auto&& matId: materialIds) {
    //     // Reuse materials defined in asset manager if possible
    //     auto mat = manager.get_material(matId);

    //     if (!mat) {
    //         continue;
    //     }
    // }
}

void Model::update_refs() {
    // Process bones
    std::for_each(bones.begin(), bones.end(), [this](std::shared_ptr<Bone> bone){
        bone->update_refs(this);
    });
    
    // Process parts
    std::for_each(parts.begin(), parts.end(), [this](std::shared_ptr<Part> part){
        part->update_refs(this);
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

void Model::link_material(const std::string materialId, const std::shared_ptr<gfx::Material> &material) {
    throw std::runtime_error("TODO link_material");
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

void gfx::zmd2mdl::from_json(const json &src, MetaUniformAny &val) {
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

void gfx::zmd2mdl::from_json(const json &src, MetaMaterialParams &val) {
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

void gfx::zmd2mdl::from_json(const json &src, MetaEmbeddedMaterial &val) {
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

std::string gfx::zmd2mdl::format_as(MetaEmbeddedTexture val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaEmbeddedMaterial val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaMaterialParams val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformSampler val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformCol val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformFloat val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformInt val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformVec val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformIvec val) { return val.to_string(); };
std::string gfx::zmd2mdl::format_as(MetaUniformMat4 val) { return val.to_string(); };