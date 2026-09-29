/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#include <zmd2/helper.hpp>
#include <zmd2/types.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;

void load(zcl::stream::byteistream &bytes, glm::vec3 &out) {
    bytes >> out.x >> out.y >> out.z;
}

void load(zcl::stream::byteistream &bytes, glm::vec4 &out) {
    bytes >> out.x >> out.y >> out.z >> out.w;
}

void load(zcl::stream::byteistream &bytes, glm::quat &out) {
    bytes >> out.x >> out.y >> out.z >> out.w;
}

void zmd2::zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes, Bbox &out) {
    float  minX = 0, minY = 0, minZ = 0,
            maxX = 0, maxY = 0, maxZ = 0;

    bytes >> minX >> minY >> minZ >> maxX >> maxY >> maxZ;
    out.min = glm::vec3(minX, minY, minZ);
    out.max = glm::vec3(maxX, maxY, maxZ);
}

void zmd2::zmd2_load_tf_from_buffer(zcl::stream::byteistream &bytes, gfx::Transform &out) {
    // Position (f32 * 3)
    load(bytes, out.pos);
    // Rotation (quat f32 * 4)
    load(bytes, out.rot);
    // Scale (f32 * 3)
    load(bytes, out.scale);
}

void zmd2::zmd2_load_bone_from_buffer(zcl::stream::byteistream &bytes, Bone &out, const std::string &id) {
    uint32_t childrenNum;

    out.id = id;

    // Parent bone index (u32), `0xffffffff` (-1) if none
    bytes >> out.parentIdx;

    // Number of children bone indices (u32)
    bytes >> childrenNum;

    // Children bone indices (u32 each)
    out.childrenIndices.resize(childrenNum);
    zmd2_load_vector(bytes, out.childrenIndices);

    // Local transform info
    zmd2_load_tf_from_buffer(bytes, out.tfLocal);

    // Length (f32)
    bytes >> out.length;
    zcl::logger("ZMD2")->info("\tBONE {0}: PARENT: {1}({1:#x}) ({2} CHILDS)",  out.id, out.parentIdx, childrenNum);
}

std::shared_ptr<Part> zmd2::zmd2_load_part_from_buffer(zcl::stream::byteistream &bytes, const std::string &id) {
    std::shared_ptr<Part> res;

    uint8_t type;
    uint32_t parentIdx;
    uint32_t childrenNum;
    std::vector<uint32_t> childrenIndices;

    // Part type (u8)
    bytes >> type;
    // Parent part index (u32), `0xffffffff` (-1) if none
    bytes >> parentIdx;
    // Number of children part indices (u32)
    bytes >> childrenNum;
    // Children part indices (u32 each)
    childrenIndices.resize(childrenNum);
    zmd2_load_vector(bytes, childrenIndices);

    zcl::logger("ZMD2")->info("\t\tPART: TYPE: {0}, PARENT: {1}({1:#x}), ({2} CHILDS)", type, parentIdx, childrenNum);

    // Type dependent data
    switch (type) {
        case ZMD2_PART_POINT: {
                gfx::Transform tfLocal;
                Bbox bounds;

                // Local transform info
                zmd2_load_tf_from_buffer(bytes, tfLocal);
                // Bounding box (f32 * 6)
                zmd2_load_bbox_from_buffer(bytes, bounds);
                
                res = std::make_shared<PartPoint>(id, parentIdx, childrenIndices, tfLocal, bounds);
                zcl::logger("ZMD2")->info("\tPART (POINT)");
            }
            break;

        case ZMD2_PART_MODEL: {
                gfx::Transform tfLocal;
                Bbox bounds;
                std::shared_ptr<gfx::VertFormat> meshFormat;
                ZMD2_MODEL meshType = ZMD2_MODEL_NONE;
                GLenum meshPrim = 0;
                char meshTypeNameBuff[64];
                std::string meshTypeName;
                uint8_t numMorphs = 0;
                uint32_t numMaterials = 0;
                std::vector<uint32_t> morphIndices;
                std::vector<uint32_t> materialIndices;
                
                // Local transform info
                zmd2_load_tf_from_buffer(bytes, tfLocal);
                // Bounding box (f32 * 6)
                zmd2_load_bbox_from_buffer(bytes, bounds);
                // Mesh / vb type name (null terminated str, 64 chars max)
                bytes.getline(meshTypeNameBuff, sizeof(meshTypeNameBuff), '\0');
                meshTypeName = std::string(meshTypeNameBuff);

                if (auto foundType = ZMD2_MODEL_FROM_NAME_TBL.find(meshTypeName); foundType != ZMD2_MODEL_FROM_NAME_TBL.end()) {
                    meshType = foundType->second;
                }
                if (auto foundFormat = ZMD2_MODEL_TO_FORMAT_TBL.find(meshType); foundFormat != ZMD2_MODEL_TO_FORMAT_TBL.end()) {
                    meshFormat = foundFormat->second;
                }
                if (auto foundPrim = ZMD2_MODEL_TO_PRIM_TBL.find(meshType); foundPrim != ZMD2_MODEL_TO_PRIM_TBL.end()) {
                    meshPrim = foundPrim->second;
                }

                if (meshType == ZMD2_MODEL_NONE || !meshFormat || !meshPrim) {
                    throw std::runtime_error(fmt::format("Unsupported mesh type ({}) '{}'!", meshTypeName.size(), meshTypeName));
                }
                
                // Number of morphs (u8)
                bytes >> numMorphs;
                // Number of used materials (u32)
                bytes >> numMaterials;
                // Morph indices (u32 each)
                morphIndices.resize(numMorphs);
                zmd2_load_vector(bytes, morphIndices);
                // Material indices (u32 each)
                materialIndices.resize(numMaterials);
                zmd2_load_vector(bytes, materialIndices);

                zcl::logger("ZMD2")->info("\t\tPART (MODEL): TYPE {} ({} MORPHS, {} MATERIALS, BOUND min {}, max {})", meshTypeName, numMorphs, numMaterials, glm::to_string(part.bounds.min), glm::to_string(part.bounds.max));

                // Mesh data per material
                auto meshGroups = std::vector<std::shared_ptr<MeshGroup>>(numMaterials);
                auto meshGroupsByMaterialIdx = std::map<uint32_t, std::shared_ptr<MeshGroup>>();

                for (auto i=0; i<numMaterials; i++) {
                    uint32_t materialIdx = 0;
                    uint32_t numVerts = 0;
                    std::shared_ptr<gfx::Vb> mesh = std::make_shared<gfx::Vb>();
                    
                    // Material index (u32, relative; from global model material table)
                    bytes >> materialIdx;
                    // Number of vertices (u32)
                    bytes >> numVerts;

                    // Vertices bytes
                    auto numBytes = numVerts * meshFormat->get_size();
                    auto vertBytes = std::vector<char>();
                    
                    vertBytes.resize(numBytes);
                    bytes.read(vertBytes.data(), numBytes);
                    mesh->set_buffer<char>(gfx::Vb::VB_BUFFER_VBO, vertBytes, meshFormat->get_size());

                    auto meshGroupPtr = std::make_shared<MeshGroup>(MeshGroup { .materialIdx = materialIdx, .mesh = mesh });
                    
                    meshGroups[i] = meshGroupPtr;
                    meshGroupsByMaterialIdx[materialIdx] = meshGroupPtr;

                    zcl::logger("ZMD2")->info("\t\tMAT {} ({} VERTS, {} BYTES, {} BYTES PER VERT)", materialIdx, numVerts, numBytes, meshFormat->get_size());
                }

                res = std::make_shared<PartModel>(id, parentIdx, childrenIndices, tfLocal, bounds, meshType, meshPrim, morphIndices, materialIndices, meshGroups, meshGroupsByMaterialIdx);
            }    
            break;

        default:
            throw std::runtime_error(fmt::format("Unsupported part type {}!", type));
            break;
    }
    return res;
}
