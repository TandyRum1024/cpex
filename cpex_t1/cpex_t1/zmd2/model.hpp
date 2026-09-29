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

#include <zmd2/types.hpp>

// LIBRARIES //
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/vb.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace zmd2 {    
    const static std::shared_ptr<gfx::VertFormat> ZMD2_VERT_FORMAT_MESH = std::make_shared<gfx::VertFormat>(
            gfx::VertFormat({
                gfx::VertAttribute(0, 3, GL_FLOAT, sizeof(float)), // POS
                gfx::VertAttribute(1, 2, GL_FLOAT, sizeof(float)), // UV
                gfx::VertAttribute(2, 3, GL_FLOAT, sizeof(float)), // NORMAL
                gfx::VertAttribute(3, 4, GL_UNSIGNED_BYTE, sizeof(uint8_t)), // COL
            })
        );
    const static std::shared_ptr<gfx::VertFormat> ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED = std::make_shared<gfx::VertFormat>(
            gfx::VertFormat({
                gfx::VertAttribute(0, 3, GL_FLOAT, sizeof(float)), // POS
                gfx::VertAttribute(1, 2, GL_FLOAT, sizeof(float)), // UV
                gfx::VertAttribute(2, 3, GL_FLOAT, sizeof(float)), // NORMAL
                gfx::VertAttribute(3, 4, GL_UNSIGNED_BYTE, sizeof(uint8_t)), // COL

                gfx::VertAttribute(4, 1, GL_FLOAT, sizeof(float)), // PART IDX
                gfx::VertAttribute(5, 4, GL_FLOAT, sizeof(float)), // BONE INDICES
                gfx::VertAttribute(6, 4, GL_FLOAT, sizeof(float)), // BONE WEIGHTS

                gfx::VertAttribute(7, 3, GL_FLOAT, sizeof(float)), // MORPH 1 POS OFF
                gfx::VertAttribute(8, 3, GL_FLOAT, sizeof(float)), // MORPH 1 NORMAL OFF
                gfx::VertAttribute(9, 3, GL_FLOAT, sizeof(float)), // MORPH 2 POS OFF
                gfx::VertAttribute(10, 3, GL_FLOAT, sizeof(float)), // MORPH 2 NORMAL OFF
            })
        );
    
    /** Zmd2: Bone. */
    class Bone {
        // Original bone data
        std::string id;
        
        uint32_t parentIdx;
        std::vector<uint32_t> childrenIndices;

        float length;
        TfData tfLocal;

        // Additional loadded data
        std::weak_ptr<Bone> parent;
        std::vector<std::shared_ptr<Bone>> children;
    
    public:
        Bone();
    };

    /** Zmd2: Model. */
    class Zmd2Model {
        std::string id;
        BboxData bounds;
        
        // Rig info.
        std::vector<Bone> bones;
        // Parts info.
        std::vector<PartData> parts;
        // Material info. (names for now)
        std::vector<std::string> materialNames;
        // Morph names.
        std::vector<std::string> morphNames;
        // Metadata. (in JSON format)
        json metadata;
        // Extra data. (bytes!)
        std::vector<uint8_t> extraBytes;

        // Internal cache
        std::vector<std::shared_ptr<gfx::Material>> linkedMaterials;
    
    public:
        Zmd2Model(std::string id);

        /** Reserve and return a new meshgroup for given material ID. */
        std::shared_ptr<MaterialAndMeshPair> reserve_meshgroup(const std::string &materialId);

        /** Find and return meshgroups for given material ID. */
        std::shared_ptr<MaterialAndMeshPair> find_meshgroup_by_material_id(const std::string &materialId) const;
        /** Submit all meshgroups to GPU. */
        void submit(gfx::TextureManager &texManager) const;
    };

    
}

#endif