/**
 * gfx_zmd2::zmd2_model - `.zmd2` model loading & wrapper for this projects graphics library.
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
#include <gfx/transform.hpp>
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/vb.hpp>
#include <zen/asset_manager.hpp>
#include <zmd2/types.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    namespace zmd2mdl {
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
        // Table to convert model type to vertex format.
        const static std::unordered_map<zmd2::MODEL_TYPE, std::shared_ptr<gfx::VertFormat>> ZMD2_FORMAT_BY_MODEL_TBL = {
            { zmd2::MODEL_TYPE_MESH, ZMD2_VERT_FORMAT_MESH },
            { zmd2::MODEL_TYPE_MESH_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
            { zmd2::MODEL_TYPE_WIRE_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
        };
        // Table to convert primitive type to OpenGL primitive mode.
        const static std::unordered_map<zmd2::PRIM_TYPE, GLenum> ZMD2_GL_PRIM_BY_PRIM_TBL = {
            { zmd2::PRIM_TYPE_TRIANGLE_LIST, GL_TRIANGLES },
            { zmd2::PRIM_TYPE_LINE_LIST, GL_LINES },
        };

        /** Returns OpenGL primitive mode from given primitive type. `0` if not found. */
        GLenum find_gl_prim_by_prim(const zmd2::PRIM_TYPE &type);
        
        /** ZMD2: Pair of material index with vertices data for mesh. */
        struct MaterialAndMeshPair {
            uint32_t materialIdx;
            std::shared_ptr<gfx::Material> material;
            std::shared_ptr<gfx::Vb> mesh;
        };

        /** ZMD2: Bounding box. */
        struct Bbox {
            glm::vec3 min;
            glm::vec3 max;
            
            // Helper constructor.
            Bbox(zmd2::BboxData src);
            Bbox();

            void merge_from(Bbox &other);
        };

        /** ZMD2: Base part. */
        struct Part {
            std::string id;
            zmd2::PART_TYPE type;
            
            uint32_t parentIdx;
            std::vector<uint32_t> childrenIndices;

            // Additional internal data...

            std::weak_ptr<Part> parent;
            std::vector<std::shared_ptr<Part>> children;

            // Helper constructor.
            Part(zmd2::PartData src);
            Part(std::string id, zmd2::PART_TYPE type);
        };

        /** ZMD2: Part (point) */
        struct PartPoint: public Part {
            gfx::Transform tfLocal;
            Bbox bounds;

            // Helper constructor.
            PartPoint(zmd2::PartPointData src);
            PartPoint(std::string id);
        };

        /** ZMD2: Part (model) */
        struct PartModel: public Part {
            gfx::Transform tfLocal;
            Bbox bounds;

            zmd2::MODEL_TYPE modelType;
            zmd2::PRIM_TYPE modelPrim;
            GLenum modelPrimGl;
            
            std::vector<uint32_t> morphIndices;
            std::vector<uint32_t> materialIndices;

            std::vector<std::shared_ptr<MaterialAndMeshPair>> matMeshes;
            // std::unordered_map<uint32_t, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialIdx;

            // Additional internal data...

            std::vector<std::string> morphNames;
            std::vector<std::shared_ptr<gfx::Material>> materials;
            std::unordered_map<std::string, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialId;

            /** Helper constructor. */
            PartModel(zmd2::PartModelData src);
            PartModel(std::string id);

            /** Reserve and return a new MaterialAndMeshPair for given material ID. */
            std::shared_ptr<MaterialAndMeshPair> reserve_matmesh(const std::string &materialId, std::shared_ptr<gfx::Material> &material);

            /** Find and return MaterialAndMeshPair for given material ID. */
            std::shared_ptr<MaterialAndMeshPair> find_matmesh_by_material_id(const std::string &materialId) const;
        };

        /** ZMD2: Bone. */
        struct Bone {
            // Original bone data
            std::string id;
            
            uint32_t parentIdx;
            std::vector<uint32_t> childrenIndices;

            float length;
            gfx::Transform tfLocal;

            // Additional internal data...

            std::weak_ptr<Bone> parent;
            std::vector<std::shared_ptr<Bone>> children;
        
            /** Helper constructor. */
            Bone(zmd2::BoneData src);
        };

        /** Embedded Material: Parameters. */
        struct MaterialParams {
            // TODO //
            // boolean alphaTest = false;
            // boolean cull = 0;
        };

        /** Embedded Material: Uniforms. */
        struct MaterialUniform {
            // TODO //
            // boolean alphaTest = false;
            // boolean cull = 0;
        };

        /** ZMD2 Metadata: Embedded material info. */
        struct MetaMaterial {
            std::string name;
            std::string baseTextureId;
            std::vector<std::string> textureIds;
            std::string shaderId;
            MaterialParams params;
            std::vector<MaterialUniform> uniforms;
        };

        /** ZMD2 Metadata: Embedded texture info. */
        struct MetaTexture {
            std::string name = "";
            bool isEmbedded = false;
            uint32_t width = 0;
            uint32_t height = 0;
            GLenum format = GL_RGBA8;
            size_t blobOffset = 0;
            size_t blobSize = 0;
        };

        /** ZMD2: Model. */
        class Model {
            std::string id;
            Bbox bounds;
            
            // Rig info.
            std::vector<std::shared_ptr<Bone>> bones;
            // Parts info.
            std::vector<std::shared_ptr<Part>> parts;
            // Material info.
            std::vector<std::string> materialIds;
            // Morph names.
            std::vector<std::string> morphNames;
            // Metadata. (in JSON format)
            // json metadata;
            // Extra data. (bytes!)
            std::vector<uint8_t> extraBytes;

            // (Metadata) Embedded materials
            std::vector<MetaMaterial> embeddedMaterials;
            // (Metadata) Embedded textures
            std::vector<MetaTexture> embeddedTextures;

            // Internal cache
            std::vector<std::shared_ptr<gfx::Material>> linkedMaterials;
            std::unordered_map<std::string, std::shared_ptr<Part>> partsById;
            std::unordered_map<std::string, std::shared_ptr<Bone>> bonesById;
        
        public:
            Model(std::string id);

            void load_embedded_materials(const zen::AssetManager &manager);
            
            void load_embedded_textures(const zen::AssetManager &manager);

            void link_material(const std::string materialId, const std::shared_ptr<gfx::Material> &material);

            // void add_material(const std::shared_ptr<gfx::Material> &material);

            void add_part(const std::shared_ptr<Part> &part);

            void add_bone(const std::shared_ptr<Bone> &bone);

            /** Find and return part for given ID. */
            template <typename T>
            std::shared_ptr<T> find_part_by_id(const std::string &id) const;

            /** Find and return bone for given ID. */
            std::shared_ptr<Bone> find_bone_by_id(const std::string &id) const;

            /** Submit all meshgroups to GPU. */
            void submit(gfx::TextureManager &texManager) const;
        };

        template <typename T>
        std::shared_ptr<T> Model::find_part_by_id(const std::string &id) const {
            auto res = partsById.find(id);
            
            if (res == partsById.end()) {
                return nullptr;
            }

            auto casted = std::static_pointer_cast<T>((*res).second);

            if (!casted) {
                return nullptr;
            }

            return casted;
        };
    }
}

#endif