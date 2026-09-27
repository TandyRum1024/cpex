/**
 * zmd2::types - Types used in `.zmd2` model.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_TYPES_GUARD
#define __ZMD2_TYPES_GUARD

#include <string>
#include <memory>
#include <vector>

// LIBRARIES //
#include <gfx/vb.hpp>
#include <gfx/material.hpp>
#include <gfx/transform.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace zmd2 {
    enum ZMD2_PART {
        ZMD2_PART_POINT = 0,
        ZMD2_PART_MODEL
    };

    enum ZMD2_MODEL {
        ZMD2_MODEL_MESH = 0,
        ZMD2_MODEL_MESH_MORPH_SKINNED,
        ZMD2_MODEL_WIRE_MORPH_SKINNED
    };

    enum ZMD2_PRIM {
        ZMD2_PRIM_TRIANGLE_LIST = 0,
        ZMD2_PRIM_LINE_LIST
    };

    struct MeshGroup {
        uint32_t materialIdx;
        std::shared_ptr<gfx::Material> material;
        std::shared_ptr<gfx::Vb> mesh;
    };

    /** Bounding box. */
    struct Bbox {
        glm::vec3 min;
        glm::vec3 max;
        
        void merge_from(Bbox &other);
    };

    /** Base part. */
    struct Part {
        std::string id;
        ZMD2_PART type;
        
        uint32_t parentIdx;

        std::vector<uint32_t> childrenIndices;
        uint32_t childrenNum;
    };

    struct PartPoint: public Part {
        ZMD2_PART type = ZMD2_PART_POINT;

        gfx::Transform tfLocal;
        Bbox bounds;
    };

    struct PartModel: public Part {
        ZMD2_PART type = ZMD2_PART_MODEL;

        gfx::Transform tfLocal;
        Bbox bounds;

        ZMD2_MODEL modelType;
        ZMD2_PRIM modelPrim;
        
        std::vector<uint32_t> morphIndices;
        std::vector<uint32_t> materialIndices;

        std::vector<std::shared_ptr<MeshGroup>> meshGroups;
        std::map<uint32_t, std::shared_ptr<MeshGroup>> meshGroupsByMaterialIdx;
    };

    /** Bone. */
    struct Bone {
        std::string id;
        
        uint32_t parentIdx;

        std::vector<uint32_t> childrenIndices;
        uint32_t childrenNum;

        double length;
        gfx::Transform tfLocal;
    };

    /** Header for `.zmd2` file. */
    struct Zmd2Header {
        // Info
        char magic[4];
        uint8_t isCompressed;

        // Directories
        uint32_t dirRigOff;
        uint32_t dirRigLen;
        uint32_t dirPartsOff;
        uint32_t dirPartsLen;
        uint32_t dirMetaOff;
        uint32_t dirMetaLen;
        uint32_t dirExtraOff;
        uint32_t dirExtraLen;

        // Bounds
        Bbox bounds;

        // Number of elements
        uint32_t numBones;
        uint32_t numParts;
        uint32_t numMaterials;
        uint32_t numMorphs;

        // Names of elements
        std::vector<std::string> nameBones;
        std::vector<std::string> nameParts;
        std::vector<std::string> nameMaterials;
        std::vector<std::string> nameMorphs;

        std::string to_string();
        bool is_valid();
    };
}
#endif