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

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //


namespace zmd2 {    
    const static char HEADER_MAGIC[4] = { 'Z', 'M', 'D', '2' };

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

    std::shared_ptr<Model> load_model_from(const std::string &id, std::istream &in, std::streampos begin = -1, std::streampos end = -1);
}

#endif