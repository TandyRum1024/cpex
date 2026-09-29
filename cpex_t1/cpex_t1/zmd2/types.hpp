/**
 * zmd2::types - Types used for loading `.zmd2` model.
 * Intended to be graphics library-agnostic, so users are reponsible to convert them to appropriate types to use it!
 * For similar reasons, all the data is represented as structs to avoid making too many getter/setters.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_TYPES_GUARD
#define __ZMD2_TYPES_GUARD

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// EXTERNAL LIBRARIES //
// ----------------------------
// JSON
#include <nlohmann/json.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using json = nlohmann::json;

namespace zmd2 {
    /** Part type. */
    enum PART_TYPE {
        PART_TYPE_POINT = 0,
        PART_TYPE_MODEL
    };
    /** Model / mesh type. */
    enum MODEL_TYPE {
        MODEL_TYPE_NONE = 0, // undefined / error
        MODEL_TYPE_MESH,
        MODEL_TYPE_MESH_MORPH_SKINNED,
        MODEL_TYPE_WIRE_MORPH_SKINNED
    };
    /** Primitive type. Analogous to GameMaker's primitive type. */
    enum PRIM_TYPE {
        PRIM_TYPE_NONE = 0,         // undefined / error
        PRIM_TYPE_TRIANGLE_LIST,    // pr_trianglelist
        PRIM_TYPE_LINE_LIST         // pr_linelist
    };

    // VERTEX LAYOUT DEFINITIONS
    #pragma pack(push, 1)
    /** Vertex layout used in basic ZMD2 models. */
    struct VertMesh {
        // Position (f32 * 3)
        float pos[3];
        // UV (f32 * 2)
        float uv[2];
        // Normal (f32 * 3)
        float normal[3];
        // Colour (RGBA, u8 * 4)
        uint8_t colRgba[4];
        // GLM (& OpenGL) representation, for reference...
        // glm::vec3 pos;
        // glm::vec2 uv;
        // glm::vec3 normal;
        // glm::u8vec3 col;
    };

    /** Vertex layout used in ZMD2 models with vertex skinning and morph. */
    struct VertMeshMorphSkinned {
        // Position (f32 * 3)
        float pos[3];
        // UV (f32 * 2)
        float uv[2];
        // Normal (f32 * 3)
        float normal[3];
        // Colour (RGBA, u8 * 4)
        uint8_t colRgba[4];
        // Part index (f32)
        float partIdx;
        // Bone indices (f32 * 4)
        float boneIndices[4];
        // Bone weights (f32 * 4)
        float boneWeights[4];
        // Morph #1 position & normal offset (f32 * 3, f32 * 3)
        float morph1PosOff[3];
        float morph1NormalOff[3];
        // Morph #2 position & normal offset (f32 * 3, f32 * 3)
        float morph2PosOff[3];
        float morph2NormalOff[3];
        // GLM (& OpenGL) representation, for reference...
        // glm::vec3 pos;
        // glm::vec2 uv;
        // glm::vec3 normal;
        // glm::u8vec3 col;
        // float partIdx;
        // glm::vec4 boneIndices;
        // glm::vec4 boneWeight;
    };
    #pragma pack(pop)

    // Table to convert names to model type.
    const static std::unordered_map<std::string, MODEL_TYPE> ZMD2_MODEL_BY_NAME_TBL = {
        { "mesh", MODEL_TYPE_MESH },
        { "mesh.morph.skinned", MODEL_TYPE_MESH_MORPH_SKINNED },
        { "wire.morph.skinned", MODEL_TYPE_WIRE_MORPH_SKINNED },
    };
    // Table to convert model type to primitive type.
    const static std::unordered_map<MODEL_TYPE, PRIM_TYPE> ZMD2_PRIM_BY_MODEL_TBL = {
        { MODEL_TYPE_MESH, PRIM_TYPE_TRIANGLE_LIST },
        { MODEL_TYPE_MESH_MORPH_SKINNED, PRIM_TYPE_TRIANGLE_LIST },
        { MODEL_TYPE_WIRE_MORPH_SKINNED, PRIM_TYPE_LINE_LIST },
    };
    // Table to convert model type to vertex strite.
    const static std::unordered_map<MODEL_TYPE, size_t> ZMD2_VERT_STRIDE_BY_NAME_TBL = {
        { MODEL_TYPE_MESH, sizeof(VertMesh) },
        { MODEL_TYPE_MESH_MORPH_SKINNED, sizeof(VertMeshMorphSkinned) },
        { MODEL_TYPE_WIRE_MORPH_SKINNED, sizeof(VertMeshMorphSkinned) },
    };
    // Table to convert model type to vertex format.
    // const static std::unordered_map<MODEL_TYPE, std::shared_ptr<gfx::VertFormat>> ZMD2_MODEL_TO_FORMAT_TBL = {
    //     { MODEL_TYPE_MESH, ZMD2_VERT_FORMAT_MESH },
    //     { MODEL_TYPE_MESH_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
    //     { MODEL_TYPE_WIRE_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
    // };

    /** Returns `MODEL_TYPE` from given name. `MODEL_TYPE_NONE` if not found. */
    MODEL_TYPE find_model_type_by_name(const std::string &name);

    /** Returns `PRIM_TYPE` from given type. `PRIM_TYPE_NONE` if not found. */
    PRIM_TYPE find_prim_type_by_model_type(const MODEL_TYPE &type);

    /** Retrurns size of vertex layout / format stride from given type. 0 if not found. */
    size_t find_vertex_stride_by_model_type(const MODEL_TYPE &type);

    /** Bounding box. */
    struct BboxData {
        float min[3];
        float max[3];
        
        void merge_from(BboxData &other);
    };

    /** Transform matrix wrapper. */
    struct TfData {
        // Position (f32 * 3)
        float pos[3];
        // Rotation (quaternion, f32 * 4)
        float rotQuaternion[4];
        // Scale (f32 * 3)
        float scale[3];
    };

    /** Pair of material index with vertices data for mesh. */
    struct MaterialAndMeshPair {
        uint32_t materialIdx;
        std::vector<uint8_t> verticesData;
        // std::shared_ptr<gfx::Material> material;
    };

    /** Loaded base part. */
    struct PartData {
        std::string id;
        PART_TYPE type;
        
        uint32_t parentIdx;
        std::vector<uint32_t> childrenIndices;

        // Helper constructor.
        PartData(std::string id, PART_TYPE type, uint32_t parentIdx, std::vector<uint32_t> childrenIndices);
    };

    /** Part: "Point" with bounds and transformation data. */
    struct PartPointData: public PartData {
        TfData tfLocal;
        BboxData bounds;

        // Helper constructor for ensuring the type is automatically assigned as correct type.
        PartPointData(std::string id, uint32_t parentIdx, std::vector<uint32_t> childrenIndices, TfData tfLocal, BboxData bounds);
    };

    /** Part: Model composed of multiple material & contains mesh data for each said material. */
    struct PartModelData: public PartData {
        TfData tfLocal;
        BboxData bounds;

        MODEL_TYPE modelType;
        PRIM_TYPE modelPrim;
        
        std::vector<uint32_t> morphIndices;
        std::vector<uint32_t> materialIndices;

        std::vector<std::shared_ptr<MaterialAndMeshPair>> matMeshes;
        std::unordered_map<uint32_t, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialIdx;

        // Helper constructor for ensuring the type is automatically assigned as correct type.
        PartModelData(
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
        );
    };

    /** Loaded bone. */
    struct BoneData {
        std::string id;
        
        uint32_t parentIdx;
        std::vector<uint32_t> childrenIndices;

        float length;
        TfData tfLocal;
    };

    /** Header for `.zmd2` file. */
    struct Header {
        // Basic info.
        char magic[4];
        uint8_t isCompressed;
        // Directories.
        uint32_t dirRigOff;
        uint32_t dirRigLen;
        uint32_t dirPartsOff;
        uint32_t dirPartsLen;
        uint32_t dirMetaOff;
        uint32_t dirMetaLen;
        uint32_t dirExtraOff;
        uint32_t dirExtraLen;
        // Bounds.
        BboxData bounds;
        // Number of elements.
        uint32_t numBones;
        uint32_t numParts;
        uint32_t numMaterials;
        uint32_t numMorphs;
        // Names of elements.
        std::vector<std::string> nameBones;
        std::vector<std::string> nameParts;
        std::vector<std::string> nameMaterials;
        std::vector<std::string> nameMorphs;

        /** Returns human readable string representation. */
        std::string to_string();

        /** Returns human readable string representation. */
        bool is_valid() const;
    };
    const static char HEADER_MAGIC[4] = { 'Z', 'M', 'D', '2' };

    /** Loaded `.zmd2` file data. Users are responsible for converting this to appropriate format! */
    struct Model {
        std::string id;
        BboxData bounds;
        
        // Rig info.
        std::vector<std::shared_ptr<BoneData>> bones;
        std::unordered_map<std::string, std::shared_ptr<BoneData>> bonesById;
        // Parts info.
        std::vector<std::shared_ptr<PartData>> parts;
        std::unordered_map<std::string, std::shared_ptr<PartData>> partsById;
        // Material info. (names for now)
        std::vector<std::string> materialNames;
        // Morph names.
        std::vector<std::string> morphNames;
        // Metadata. (in JSON format)
        json metadata;
        // Extra data. (bytes!)
        std::vector<uint8_t> extraBytes;
    };
}
#endif