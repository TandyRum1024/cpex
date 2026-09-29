/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#include <unordered_map>

#include <zmd2/helper.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// fmt
#include <fmt/ranges.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;

template <typename T>
void load(zcl::stream::byteistream &bytes, T* out, size_t len) {
    for (auto i=0; i<len; i++) {
        bytes >> out[i];
    }
}

void zmd2::zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes, BboxData &out) {
    load(bytes, out.min, 3);
    load(bytes, out.max, 3);
}

void zmd2::zmd2_load_tf_from_buffer(zcl::stream::byteistream &bytes, TfData &out) {
    // Position (f32 * 3)
    load(bytes, out.pos, 3);
    // Rotation (quat f32 * 4)
    load(bytes, out.rotQuaternion, 4);
    // Scale (f32 * 3)
    load(bytes, out.scale, 3);
}

void zmd2::zmd2_load_bone_from_buffer(zcl::stream::byteistream &bytes, std::shared_ptr<BoneData> &out, const std::string &id) {
    uint32_t parentIdx;
    uint32_t childrenNum;
    std::vector<uint32_t> childrenIndices;
    TfData tfLocal;
    float length;

    // Parent bone index (u32), `0xffffffff` (-1) if none
    bytes >> parentIdx;

    // Number of children bone indices (u32)
    bytes >> childrenNum;

    // Children bone indices (u32 each)
    childrenIndices.resize(childrenNum);
    zmd2_load_vector(bytes, childrenIndices);

    // Local transform info
    zmd2_load_tf_from_buffer(bytes, tfLocal);

    // Length (f32)
    bytes >> length;

    out = std::make_shared<BoneData>(
        BoneData {
            .id = id,
            .parentIdx = parentIdx,
            .childrenIndices = std::move(childrenIndices),
            .tfLocal = tfLocal
        }
    );
    // zcl::logger("ZMD2")->info("\tBONE {0}: PARENT: {1}({1:#x}) ({2} CHILDS)",  out.id, out.parentIdx, childrenNum);
}

void zmd2::zmd2_load_part_from_buffer(zcl::stream::byteistream &bytes, std::shared_ptr<PartData> &out, const std::string &id) {
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

    // zcl::logger("ZMD2")->info("\t\tPART: TYPE: {0}, PARENT: {1}({1:#x}), ({2} CHILDS)", type, parentIdx, childrenNum);

    // Type dependent data
    switch (type) {
        case PART_TYPE_POINT: {
                TfData tfLocal;
                BboxData bounds;

                // Local transform info
                zmd2_load_tf_from_buffer(bytes, tfLocal);
                // Bounding box (f32 * 6)
                zmd2_load_bbox_from_buffer(bytes, bounds);
                
                out = std::make_shared<PartPointData>(id, parentIdx, std::move(childrenIndices), tfLocal, bounds);
                // zcl::logger("ZMD2")->info("\tPART (POINT)");
            }
            break;

        case PART_TYPE_MODEL: {
                TfData tfLocal;
                BboxData bounds;
                MODEL_TYPE meshType = MODEL_TYPE_NONE;
                PRIM_TYPE meshPrim = PRIM_TYPE_NONE;
                size_t meshVertStride = 0;
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
                char meshTypeNameBuff[64];
                
                bytes.getline(meshTypeNameBuff, sizeof(meshTypeNameBuff), '\0');
                meshTypeName = std::string(meshTypeNameBuff);
                meshType = find_model_type_by_name(meshTypeName);
                meshPrim = find_prim_type_by_model_type(meshType);
                meshVertStride = find_vertex_stride_by_model_type(meshType);

                if (meshType == MODEL_TYPE_NONE || meshPrim == PRIM_TYPE_NONE || meshVertStride == 0) {
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

                // Mesh data per material
                auto matMeshes = std::vector<std::shared_ptr<MaterialAndMeshPair>>(numMaterials);
                auto matMeshesByMaterialIdx = std::unordered_map<uint32_t, std::shared_ptr<MaterialAndMeshPair>>();

                for (auto i=0; i<numMaterials; i++) {
                    uint32_t materialIdx = 0;
                    uint32_t numVerts = 0;
                    std::vector<uint8_t> vertsData;
                    
                    // Material index (u32, relative; from global model material table)
                    bytes >> materialIdx;
                    // Number of vertices (u32)
                    bytes >> numVerts;

                    // Vertices bytes
                    auto numBytes = numVerts * meshVertStride;
                    
                    vertsData.resize(numBytes);
                    bytes.read(reinterpret_cast<char*>(vertsData.data()), numBytes);

                    auto matMeshPtr = std::make_shared<MaterialAndMeshPair>(MaterialAndMeshPair { .materialIdx = materialIdx, .verticesData = std::move(vertsData) });

                    matMeshes[i] = matMeshPtr;
                    matMeshesByMaterialIdx[materialIdx] = matMeshPtr;

                    // zcl::logger("ZMD2")->info("\t\tMAT {} ({} VERTS, {} BYTES, {} BYTES PER VERT)", materialIdx, numVerts, numBytes, meshVertStride);
                }

                out = std::make_shared<PartModelData>(id, parentIdx, std::move(childrenIndices), tfLocal, bounds, meshType, meshPrim, std::move(morphIndices), std::move(materialIndices), std::move(matMeshes), std::move(matMeshesByMaterialIdx));
                // zcl::logger("ZMD2")->info("\t\tPART (MODEL): TYPE: {} ({} MORPHS, {} MATERIALS, BOUND: (min {}, max {}))", meshTypeName, numMorphs, numMaterials, fmt::join(bounds.min, ", "), fmt::join(bounds.max, ", "));
            }
            break;

        default:
            throw std::runtime_error(fmt::format("Unsupported part type {}!", type));
            break;
    }
}
