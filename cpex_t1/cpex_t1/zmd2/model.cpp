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
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;

std::string Zmd2Header::to_string() {
    std::stringstream stream;
    std::string magicStr = std::string(magic, 4);
    std::string magicSrcStr = std::string(HEADER_MAGIC, 4);

    stream << "ZMD2 HEADER" << std::endl;
    stream << fmt::format("\tMAGIC: {:4s} vs {:4s} (compare: {})", magicSrcStr, magicStr, std::strncmp(HEADER_MAGIC, magic, 4)) << std::endl;
    stream << fmt::format("\tCOMPRESSED: {}", isCompressed) << std::endl;
    stream << fmt::format("\tDIRECTORY: rig({0:#X}, {1}) / parts({2:#X}, {3}) / meta({4:#X}, {5}) / extra({6:#X}, {7})", dirRigOff, dirRigLen, dirPartsOff, dirPartsLen, dirMetaOff, dirMetaLen, dirExtraOff, dirExtraLen) << std::endl;
    
    stream << fmt::format("\tBONES ({}): {}", numBones, zcl::str::to_str<std::string>(nameBones, ", ")) << std::endl;
    stream << fmt::format("\tPARTS ({}): {}", numParts, zcl::str::to_str<std::string>(nameParts, ", ")) << std::endl;
    stream << fmt::format("\tMATERIALS ({}): {}", numMaterials, zcl::str::to_str<std::string>(nameMaterials, ", ")) << std::endl;
    stream << fmt::format("\tMORPHS ({}): {}", numMorphs, zcl::str::to_str<std::string>(nameMorphs, ", ")) << std::endl;

    return stream.str();
}

bool Zmd2Header::is_valid() {
    return std::strncmp(HEADER_MAGIC, magic, 4) == 0;
}

Model::Model(std::string id):
    id(id) {}

std::shared_ptr<MeshGroup> Model::reserve_meshgroup(const std::string &materialId) {
    if (meshGroupsByMaterialId.contains(materialId)) {
        return meshGroupsByMaterialId.at(materialId);
    }

    zcl::logger("ZMD2")->warn("MODEL {} MESHGROUP RESERVING => {}", id, materialId);

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

void Model::submit(gfx::TextureManager &texManager) const {
    // zcl::logger("ZMD2")->warn("SUBMIT MODEL {}...", id);

    for (auto &&meshGroup: meshGroups) {
        auto material = meshGroup->material;
        auto mesh = meshGroup->mesh;

        if (!material || !mesh) {
            if (material) {
                zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH FOR MESHGROUP {}!", id, material->get_id());
            }
            else {
                zcl::logger("ZMD2")->error("MODEL {} HAS NO MESH OR MATERIAL!", id);
            }
            continue;
        }

        // zcl::logger("ZMD2")->warn("MODEL {} SUBMIT! ({}, {} verts)", id, material->get_id(), mesh->get_indices_num());
        material->apply_material(texManager);
        mesh->submit(GL_TRIANGLES, 0);
    }
}

std::shared_ptr<Model> zmd2::load_model_from(const std::string &id, std::istream &in, std::streampos begin, std::streampos end) {
    Zmd2Header header = {};
    auto model = Model(id);
    auto seekPrev = in.tellg();
    
    // Convert for easy bytes reading
    zcl::stream::byteistream bytes = zcl::stream::byteistream(in.rdbuf());

    if (begin == -1) {
        begin = 0;
    }
    if (end == -1) {
        bytes.seekg(0, std::ios_base::end);
        end = bytes.tellg();
    }
    bytes.seekg(begin, std::ios_base::beg);
    zcl::logger("ZMD2")->warn("FILE SZ: {} ({}-{})", end - begin, 0 + begin, 0 + end);

    // Read header
    bytes.read(header.magic, 4);
    if (!header.is_valid()) {
        std::string magicStr = std::string(header.magic, 4);
        std::string magicSrcStr = std::string(HEADER_MAGIC, 4);

        throw std::runtime_error(fmt::format("ZMD2 header magic mismatch! -> {} vs {}", magicStr, magicSrcStr));
    }
    bytes >> header.isCompressed;

    // (replace to inflating stream if compressed flag is set)
    std::shared_ptr<zcl::zlib::inflated_streambuf> inflatedStreamBuf = std::shared_ptr<zcl::zlib::inflated_streambuf>(nullptr);

    if (header.isCompressed) {
        inflatedStreamBuf = std::make_shared<zcl::zlib::inflated_streambuf>(zcl::zlib::inflated_streambuf(in.rdbuf(), in.tellg(), end));
        bytes.set_rdbuf(&(*inflatedStreamBuf));
    }

    // Read offsets and sizes for each directories (u32 each)
    bytes >> header.dirRigOff;
    bytes >> header.dirRigLen;
    bytes >> header.dirPartsOff;
    bytes >> header.dirPartsLen;
    bytes >> header.dirMetaOff;
    bytes >> header.dirMetaLen;
    bytes >> header.dirExtraOff;
    bytes >> header.dirExtraLen;

    // zcl::logger("ZMD2")->info("\t(SEEKPOS: {})", 0 + bytes.tellg());

    // Bounding box (f32 * 6)
    zmd2_load_bbox_from_buffer(bytes, header.bounds);
    zcl::logger("ZMD2")->info("\tBOUNDS: min {}, max {}", glm::to_string(header.bounds.min), glm::to_string(header.bounds.max));

    // Number of bones, parts, materials, morphs (u32 * 4)
    bytes >> header.numBones;
    bytes >> header.numParts;
    bytes >> header.numMaterials;
    bytes >> header.numMorphs;

    // zcl::logger("ZMD2")->info("\t(SEEKPOS: {})", 0 + bytes.tellg());

    // Names of elements
    header.nameBones.resize(header.numBones);
    header.nameParts.resize(header.numParts);
    header.nameMaterials.resize(header.numMaterials);
    header.nameMorphs.resize(header.numMorphs);

    zmd2_load_vector(bytes, header.nameBones);
    zmd2_load_vector(bytes, header.nameParts);
    zmd2_load_vector(bytes, header.nameMaterials);
    zmd2_load_vector(bytes, header.nameMorphs);

    zcl::logger("ZMD2")->info("\tHEADER: \n" + header.to_string());

    // Read body
    
    // Bones data
    std::map<std::string, Bone> bonesData;

    zcl::logger("ZMD2")->info("\tREADING BONES");
    // bytes.seekg(header.dirRigOff, std::ios_base::beg);
    for (auto&& boneId: header.nameBones) {
        auto bone = bonesData[boneId];
        
        zmd2_load_bone_from_buffer(bytes, bone, boneId);
    }
    
    // Parts & mesh info
    std::map<std::string, std::shared_ptr<Part>> partsData;

    zcl::logger("ZMD2")->info("\tREADING PARTS");
    // bytes.seekg(header.dirPartsOff, std::ios_base::beg);

    for (auto&& partId: header.nameParts) {
        auto part = partsData[partId];
        
        part = zmd2_load_part_from_buffer(bytes, partId);
        // zcl::logger("ZMD2")->info("\tPART {}", part.id);
    }

    // Metadata
    

    return std::make_shared<Model>(model);
}
