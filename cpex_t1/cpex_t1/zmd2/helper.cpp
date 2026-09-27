/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#include <zmd2/helper.hpp>
#include <zmd2/types.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

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

void zmd2::zmd2_load_bone_from_buffer(zcl::stream::byteistream &bytes, Bone &out) {
    // Parent bone index (u32), `0xffffffff` (-1) if none
    bytes >> out.parentIdx;

    // Number of children bone indices (u32)
    bytes >> out.childrenNum;
    // Children bone indices (u32 each)
    out.childrenIndices.resize(out.childrenNum);
    zmd2_load_vector(bytes, out.childrenIndices);

    // Local transform info
    zmd2_load_tf_from_buffer(bytes, out.tfLocal);

    // Length (f32)
    bytes >> out.length;
}

void zmd2::zmd2_load_tf_from_buffer(zcl::stream::byteistream &bytes, gfx::Transform &out) {
    // Position (f32 * 3)
    load(bytes, out.pos);
    // Rotation (quat f32 * 4)
    load(bytes, out.rot);
    // Scale (f32 * 3)
    load(bytes, out.scale);
}

std::shared_ptr<Part> zmd2::zmd2_load_part_from_buffer(zcl::stream::byteistream &bytes) {
    std::shared_ptr<Part> res = std::make_shared<Part>(nullptr);

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

    // Type dependent data
    switch (type) {
        case ZMD2_PART_POINT: {
                auto part = PartPoint();
                
                // Local transform info
                zmd2_load_tf_from_buffer(bytes, part.tfLocal);
                // Bounding box (f32 * 6)
                zmd2_load_bbox_from_buffer(bytes, part.bounds);
                
                res = std::make_shared<PartPoint>(std::move(part));
            }
            break;

        case ZMD2_PART_MODEL: {
                auto part = PartModel();
                std::shared_ptr<gfx::VertFormat> meshFormat;
                ZMD2_MODEL meshType;
                std::string meshTypeName;
                uint8_t numMorphs = 0;
                uint32_t numMaterials = 0;
                
                // Local transform info
                zmd2_load_tf_from_buffer(bytes, part.tfLocal);
                // Bounding box (f32 * 6)
                zmd2_load_bbox_from_buffer(bytes, part.bounds);
                // Mesh / vb type name (null terminated str, 64 chars max)
                bytes.getline(meshTypeName.data(), 64, '\0');
                if (auto foundType = ZMD2_MODEL_FROM_NAME_TBL.find(meshTypeName); foundType != ZMD2_MODEL_FROM_NAME_TBL.end()) {
                    meshType = foundType->second;
                }
                if (auto foundFormat = ZMD2_MODEL_TO_FORMAT_TBL.find(meshType); foundFormat != ZMD2_MODEL_TO_FORMAT_TBL.end()) {
                    meshFormat = foundFormat->second;
                }
                else {
                    throw std::runtime_error(fmt::format("Unsupported mesh type {}!", meshTypeName));
                }
                // Number of morphs (u8)
                bytes >> numMorphs;
                // Number of used materials (u32)
                bytes >> numMaterials;
                // Morph indices (u32 each)
                part.morphIndices.resize(numMorphs);
                zmd2_load_vector(bytes, part.morphIndices);
                // Material indices (u32 each)
                part.materialIndices.resize(numMaterials);
                zmd2_load_vector(bytes, part.materialIndices);

                // Mesh data per material
                // for (auto i=0; i<numMaterials; i++) {
                //     uint32_t materialIdx = 0;
                //     uint32_t numVerts = 0;
                //     std::shared_ptr<gfx::Vb> mesh = std::make_shared<gfx::Vb>();
                //     MeshGroup meshGroup = MeshGroup { .mesh = mesh };
                    
                //     // Material index (u32, relative; from global model material table)
                //     bytes >> materialIdx;
                //     meshGroup.materialIdx = materialIdx;
                //     // Number of vertices (u32)
                //     bytes >> numVerts;

                //     // Vertices bytes
                //     auto numBytes = numVerts * meshFormat->get_size();
                //     std::vector<uint8_t> bytes = std::vector<uint8_t>(numBytes);

                //     mesh->set_buffer<uint8_t>(gfx::Vb::VB_BUFFER_VBO, bytes);
                // }
                
                res = std::make_shared<PartModel>(std::move(part));
            }    
            break;

        default:
            throw std::runtime_error(fmt::format("Unsupported part type {}!", type));
            break;
    }

    return res;
}
