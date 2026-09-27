/**
 * zmd2::types - Types used in `.zmd2` model.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_TYPES_GUARD
#define __ZMD2_TYPES_GUARD

#include <string>
#include <memory>

// LIBRARIES //
#include <gfx/vb.hpp>
#include <gfx/material.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace zmd2 {
    struct MeshGroup {
        std::shared_ptr<gfx::Material> material;
        std::shared_ptr<gfx::Vb> mesh;
    };

    /** Contains data for `.zmd2` formatted model. */
    struct Bbox {
        glm::vec3 min;
        glm::vec3 max;
        
        void merge_from(Bbox &other);
    };

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