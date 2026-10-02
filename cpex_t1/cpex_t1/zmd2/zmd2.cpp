/**
 * zmd2 - Zmd2 file format.
 * ZIK@MMXXVI
 */

#include <vector>
#include <unordered_map>

#include <zmd2/zmd2.hpp>
#include <zmd2/helper.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>
#include <zcl/zlib.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// JSON
#include <nlohmann/json.hpp>
// fmt
#include <fmt/ranges.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;
using json = nlohmann::json;

std::shared_ptr<Model> zmd2::load_model_from(const std::string &id, std::istream &in, std::streampos begin, std::streampos end) {
    Header header = {};
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
    // zcl::logger("ZMD2")->warn("FILE SZ: {} ({}-{})", end - begin, 0 + begin, 0 + end);

    // Read header
    bytes.read(header.magic, 4);
    if (!header.is_valid()) {
        std::string magicStr = std::string(header.magic, 4);
        std::string magicSrcStr = std::string(HEADER_MAGIC, 4);

        // throw std::runtime_error(fmt::format("ZMD2 header magic mismatch! -> {} vs {}", magicStr, magicSrcStr));
        throw std::runtime_error("ZMD2 header magic mismatch!");
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

    // Read body
    
    // Bones data
    std::unordered_map<std::string, std::shared_ptr<BoneData>> bonesDataById;
    std::vector<std::shared_ptr<BoneData>> bonesData;

    // zcl::logger("ZMD2")->info("\tREADING BONES");
    // bytes.seekg(header.dirRigOff, std::ios_base::beg);
    bonesData.resize(header.numBones);
    for (auto i=0; i<header.numBones; i++) {
        auto& boneId = header.nameBones[i];
        auto& bone = bonesDataById[boneId];
        
        zmd2_load_bone_from_buffer(bytes, bone, boneId);
        bonesData[i] = bone;
    }
    
    // Parts & mesh info
    std::unordered_map<std::string, std::shared_ptr<PartData>> partsDataById;
    std::vector<std::shared_ptr<PartData>> partsData;

    // zcl::logger("ZMD2")->info("\tREADING PARTS");
    // bytes.seekg(header.dirPartsOff, std::ios_base::beg);
    partsData.resize(header.numParts);
    for (auto i=0; i<header.numParts; i++) {
        auto& partId = header.nameParts[i];
        auto& part = partsDataById[partId];
        
        zmd2_load_part_from_buffer(bytes, part, partId);
        partsData[i] = part;
    }

    // Metadata
    std::string metaStr;
    json metadata;

    metaStr.resize(header.dirMetaLen, '\0');
    bytes.read(metaStr.data(), header.dirMetaLen);

    metadata = json::parse(metaStr);

    // Extra bytes / blob
    auto blobBytes = std::vector<uint8_t>(header.dirExtraLen);

    bytes.read(reinterpret_cast<char*>(blobBytes.data()), header.dirExtraOff);

    // Assemble the model now
    auto model = Model {
        .id = id,
        .bounds = header.bounds,
        
        .bones = std::move(bonesData),
        .parts = std::move(partsData),
        .materialNames = std::move(header.nameMaterials),
        .morphNames = std::move(header.nameMorphs),
        .metadata = std::move(metadata),
        .extraBytes = std::move(blobBytes),
    };

    // zcl::logger("ZMD2")->info("HEADER: \n" + header.to_string());
    // zcl::logger("ZMD2")->info("BODY:");
    // zcl::logger("ZMD2")->info("\tBONES:");
    // for (auto&& bone: model.bones) {
    //     zcl::logger("ZMD2")->info("\t\t{0}: PARENT: {1}({1:#x}) ({2} CHILDS)",  bone->id, bone->parentIdx, bone->childrenIndices.size());
    // }
    // zcl::logger("ZMD2")->info("\tPARTS:");
    // for (auto&& part: model.parts) {
    //     zcl::logger("ZMD2")->info("\t\t{0}: TYPE: {1}, PARENT: {2}({2:#x}), ({2} CHILDS)", part->id, static_cast<int>(part->type), part->parentIdx, part->childrenIndices.size());
        
    //     switch (part->type) {
    //         case PART_TYPE_POINT: 
    //             zcl::logger("ZMD2")->info("\t\t(TYPE POINT)");
    //             break;
    //         case PART_TYPE_MODEL:
    //             if (auto partModel = std::static_pointer_cast<PartModelData>(part)) {
    //                 zcl::logger("ZMD2")->info("\t\t(TYPE MODEL) MESH TYPE: {} ({} MORPHS, {} MATERIALS, BOUND: (min {}, max {}))", static_cast<int>(partModel->modelType), partModel->morphIndices.size(), partModel->materialIndices.size(), fmt::join(partModel->bounds.min, ", "), fmt::join(partModel->bounds.max, ", "));
    //             }
    //             break;
    //     }
    // }
    // zcl::logger("ZMD2")->info("METADATA: {}", model.metadata.dump(1));
    // zcl::logger("ZMD2")->info("EXTRA BYTES: ({} bytes)", model.extraBytes.size());

    // Seet to previous position
    in.seekg(seekPrev);

    return std::make_shared<Model>(model);
}
