/**
 * zmd2::types - Types used in `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <zmd2/types.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// fmt
#include <fmt/ranges.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zmd2;

MODEL_TYPE zmd2::find_model_type_by_name(const std::string &name) {
    if (auto found = ZMD2_MODEL_BY_NAME_TBL.find(name); found != ZMD2_MODEL_BY_NAME_TBL.end()) {
        return found->second;
    }
    return MODEL_TYPE_NONE;
}

PRIM_TYPE zmd2::find_prim_type_by_model_type(const MODEL_TYPE &type) {
    if (auto found = ZMD2_PRIM_BY_MODEL_TBL.find(type); found != ZMD2_PRIM_BY_MODEL_TBL.end()) {
        return found->second;
    }
    return PRIM_TYPE_NONE;
}

size_t zmd2::find_vertex_stride_by_model_type(const MODEL_TYPE &type) {
    if (auto found = ZMD2_VERT_STRIDE_BY_NAME_TBL.find(type); found != ZMD2_VERT_STRIDE_BY_NAME_TBL.end()) {
        return found->second;
    }
    return 0;
}

void BboxData::merge_from(BboxData &other) {
    for (auto i=0; i<3; i++) {
        min[i] = std::min(min[i], other.min[i]);
        max[i] = std::max(max[i], other.max[i]);
    }
}

// (assuming that this constructor is being called from loading routine; i.e. all the parameters are temporary/throwaway, therefore move semantics are preferred for preventing redudant copy.)
PartData::PartData(std::string id, PART_TYPE type, uint32_t parentIdx, std::vector<uint32_t> childrenIndices):
    id(id),
    type(type),
    parentIdx(parentIdx),
    childrenIndices(std::move(childrenIndices)) {}

PartPointData::PartPointData(std::string id, uint32_t parentIdx, std::vector<uint32_t> childrenIndices, TfData tfLocal, BboxData bounds):
    PartData(id, PART_TYPE_POINT, parentIdx, std::move(childrenIndices)),
    tfLocal(tfLocal),
    bounds(bounds) {}

PartModelData::PartModelData(
    std::string id,
    uint32_t parentIdx,
    std::vector<uint32_t> childrenIndices,
    TfData tfLocal,
    BboxData bounds,
    MODEL_TYPE modelType,
    PRIM_TYPE modelPrim,
    std::vector<uint32_t> morphIndices,
    std::vector<uint32_t> materialIndices,
    std::vector<std::shared_ptr<MaterialAndMeshPair>> matMeshes,
    std::unordered_map<uint32_t, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialIdx
):
    PartData(id, PART_TYPE_MODEL, parentIdx, std::move(childrenIndices)),
    tfLocal(tfLocal),
    bounds(bounds),
    modelType(modelType),
    modelPrim(modelPrim),
    morphIndices(std::move(morphIndices)),
    materialIndices(std::move(materialIndices)),
    matMeshes(std::move(matMeshes)),
    matMeshesByMaterialIdx(std::move(matMeshesByMaterialIdx)) {}

std::string Header::to_string() {
    std::stringstream stream;
    std::string magicStr = std::string(magic, 4);
    std::string magicSrcStr = std::string(HEADER_MAGIC, 4);

    stream << "ZMD2 HEADER" << std::endl;
    stream << fmt::format("\tMAGIC: {:4s} vs {:4s} (compare: {})", magicSrcStr, magicStr, std::strncmp(HEADER_MAGIC, magic, 4)) << std::endl;
    stream << fmt::format("\tCOMPRESSED: {}", isCompressed) << std::endl;
    stream << fmt::format("\tDIRECTORY: rig({0:#X}, {1}) / parts({2:#X}, {3}) / meta({4:#X}, {5}) / extra({6:#X}, {7})", dirRigOff, dirRigLen, dirPartsOff, dirPartsLen, dirMetaOff, dirMetaLen, dirExtraOff, dirExtraLen) << std::endl;

    stream << fmt::format("\tBOUNDS (min {}), (max {})", fmt::join(bounds.min, bounds.min + 3, ", "), fmt::join(bounds.max, bounds.max + 3, ", ")) << std::endl;
    stream << fmt::format("\tBONES ({}): {}", numBones, fmt::join(nameBones.begin(), nameBones.end(), ", ")) << std::endl;
    stream << fmt::format("\tPARTS ({}): {}", numParts, fmt::join(nameParts.begin(), nameParts.end(), ", ")) << std::endl;
    stream << fmt::format("\tMATERIALS ({}): {}", numMaterials, fmt::join(nameMaterials.begin(), nameMaterials.end(), ", ")) << std::endl;
    stream << fmt::format("\tMORPHS ({}): {}", numMorphs, fmt::join(nameMorphs.begin(), nameMorphs.end(), ", ")) << std::endl;

    return stream.str();
}

bool Header::is_valid() const {
    return std::strncmp(HEADER_MAGIC, magic, 4) == 0;
}