/**
 * zmd2::model - `.zmd2` model.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_MDL_GUARD
#define __ZMD2_MDL_GUARD

#include <string>
#include <memory>
#include <vector>
#include <map>

// LIBRARIES //
#include <gfx/vb.hpp>
#include <gfx/material.hpp>

namespace zmd2 {
    struct MeshGroup {
        std::shared_ptr<gfx::Material> material;
        std::shared_ptr<gfx::Vb> mesh;
    };

    /** Contains data for `.zmd2` formatted model. */
    class Model {
        std::string id;

        // (a model may contain multiple meshes grouped by mesh!)

        std::vector<std::shared_ptr<MeshGroup>> meshGroups;
        // std::vector<std::shared_ptr<gfx::Material>> materials;
        // std::vector<std::shared_ptr<gfx::Vb>> meshes;

        std::map<std::string, std::shared_ptr<MeshGroup>> meshGroupsByMaterialId;
    
    public:
        std::shared_ptr<MeshGroup> reserve_meshgroup(const std::string &materialId);
        std::shared_ptr<MeshGroup> find_meshgroup_by_material_id(const std::string &materialId) const;
        void submit(gfx::TextureManager texManager) const;
    };
}

#endif