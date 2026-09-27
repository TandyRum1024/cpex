/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_MDL_GUARD
#define __ZMD2_MDL_GUARD

#include <istream>
#include <string>
#include <memory>
#include <vector>
#include <map>

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
    const static char HEADER_MAGIC[4] = { 'Z', 'M', 'D', '2' };

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

    /** Contains data for `.zmd2` formatted model. */
    class Model {
        std::string id;

        // (a model may contain multiple meshes grouped by mesh!)

        // std::vector<std::shared_ptr<gfx::Material>> materials;
        // std::vector<std::shared_ptr<gfx::Vb>> meshes;

        std::vector<std::shared_ptr<MeshGroup>> meshGroups;
        std::map<std::string, std::shared_ptr<MeshGroup>> meshGroupsByMaterialId;
    
    public:
        Model(std::string id);

        /** Reserve and return a new meshgroup for given material ID. */
        std::shared_ptr<MeshGroup> reserve_meshgroup(const std::string &materialId);
        /** Find and return meshgroups for given material ID. */
        std::shared_ptr<MeshGroup> find_meshgroup_by_material_id(const std::string &materialId) const;
        /** Submit all meshgroups to GPU. */
        void submit(gfx::TextureManager &texManager) const;
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

    std::shared_ptr<Model> load_model_from(const std::string &id, std::istream &in, std::streampos begin = -1, std::streampos end = -1);
}

#endif