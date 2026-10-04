/**
 * gfx::zmd2::model - `.zmd2` model loading & wrapper for this projects graphics library.
 * ZIK@MMXXVI
 */

#ifndef __GFX_ZMD2_MDL_GUARD
#define __GFX_ZMD2_MDL_GUARD

#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <map>

#include <gfx/zmd2/config.hpp>
#include <gfx/transform.hpp>
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/vb.hpp>

// LIBRARIES //
#include <zmd2/types.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
// JSON
#include <nlohmann/json.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

// Debug
#define ZMD2_DEBUG_TRACE_FUNC(level, ...) if (ZMD2_DEBUG_TRACE) { zcl::logger("ZMD2")->level(__VA_ARGS__); }

// JSON deserialization helpers
// in form of `src.at("<name>").get_to(val.<name>);`
#define __ZMD2_TO_JSON_V(src, val, name) src.at(#name).get_to(val.name)
#define __ZMD2_TO_JSON_NULL(src, val, name) json_get_to(src, #name, val.name)

using json = nlohmann::json;

namespace gfx {
    namespace zmd2 {
        // Default vertex formats
        const extern std::shared_ptr<gfx::VertFormat> ZMD2_VERT_FORMAT_MESH;
        const extern std::shared_ptr<gfx::VertFormat> ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED;

        /** Material parameter: backface culling mode. */
        enum ZMD2_CULL {
            ZMD2_CULL_NOCULLING,        // cull_noculling
            ZMD2_CULL_CLOCKWISE,        // cull_clockwise
            ZMD2_CULL_COUNTERCLOCKWISE, // cull_counterclockwise
        };

        /** Material parameter: ztest equation. */
        enum ZMD2_ZFUNC {
            ZMD2_ZFUNC_NEVER,           // cmpfunc_never
            ZMD2_ZFUNC_LESS,            // cmpfunc_less
            ZMD2_ZFUNC_EQUAL,           // cmpfunc_equal
            ZMD2_ZFUNC_LESSEQUAL,       // cmpfunc_lessequal
            ZMD2_ZFUNC_GREATER,         // cmpfunc_greater
            ZMD2_ZFUNC_NOTEQUAL,        // cmpfunc_notequal
            ZMD2_ZFUNC_GREATEREQUAL,    // cmpfunc_greaterequal
            ZMD2_ZFUNC_ALWAYS,          // cmpfunc_always
        };

        /** Material parameter: blendmode. */
        enum ZMD2_BM {
            ZMD2_BM_ZERO,           // bm_zero
            ZMD2_BM_ONE,            // bm_one
            ZMD2_BM_SRC_COL,        // bm_src_colour
            ZMD2_BM_INV_SRC_COL,    // bm_inv_src_colour
            ZMD2_BM_SRC_ALPHA,      // bm_src_alpha
            ZMD2_BM_INV_SRC_ALPHA,  // bm_inv_src_alpha
            ZMD2_BM_DST_COL,        // bm_dest_colour
            ZMD2_BM_INV_DST_COL,    // bm_inv_dest_colour
            ZMD2_BM_DST_ALPHA,      // bm_dest_alpha
            ZMD2_BM_INV_DST_ALPHA,  // bm_inv_dest_alpha
            ZMD2_BM_SRC_ALPHA_SAT,  // bm_src_alpha_sat
        };

        /** Material parameter: blendmode equation. */
        enum ZMD2_BM_EQ {
            ZMD2_BM_EQ_ADD,                 // bm_eq_add
            ZMD2_BM_EQ_SUBTRACT,            // bm_eq_subtract
            ZMD2_BM_EQ_REVERSE_SUBTRACT,    // bm_eq_reverse_subtract
            ZMD2_BM_EQ_MIN,                 // bm_eq_min
            ZMD2_BM_EQ_MAX,                 // bm_eq_max
        };

        /** Texture: format. */
        enum ZMD2_TEX_FORMAT {
            ZMD2_TEX_FORMAT_RGBA8_UNORM,    // surface_rgba8unorm
            ZMD2_TEX_FORMAT_R8_UNORM,       // surface_r8unorm
            ZMD2_TEX_FORMAT_RG8_UNORM,      // surface_rg8unorm
            ZMD2_TEX_FORMAT_RGBA4_UNORM,    // surface_rgba4unorm
            ZMD2_TEX_FORMAT_RGBA16_FLOAT,   // surface_rgba16float
            ZMD2_TEX_FORMAT_R16_FLOAT,      // surface_r16float
            ZMD2_TEX_FORMAT_RGBA32_FLOAT,   // surface_rgba32float
            ZMD2_TEX_FORMAT_R32_FLOAT,      // surface_r32float
        };

        // (forward decls)
        
        class Model;
        
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
            Bbox(::zmd2::BboxData *src);
            Bbox();

            void merge_from(Bbox &other);
        };

        /** ZMD2: Base part. */
        struct Part {
            std::string id;
            ::zmd2::PART_TYPE type;
            
            uint32_t parentIdx;
            std::vector<uint32_t> childrenIndices;

            // Additional internal data...

            std::weak_ptr<Part> parent;
            std::vector<std::shared_ptr<Part>> children;

            // Helper constructor.
            Part(::zmd2::PartData *src);
            Part(std::string id, ::zmd2::PART_TYPE type);

            /** Updates internal references from given model which contains the fully loaded data. */
            virtual void update_refs(Model *mdl);
        };

        /** ZMD2: Part (point) */
        struct PartPoint: public Part {
            gfx::Transform tfLocal;
            Bbox bounds;

            // Helper constructor.
            PartPoint(::zmd2::PartPointData *src);
            PartPoint(std::string id);

            // void update_refs(Model *mdl);
        };

        /** ZMD2: Part (model) */
        struct PartModel: public Part {
            gfx::Transform tfLocal;
            Bbox bounds;

            ::zmd2::MODEL_TYPE modelType;
            ::zmd2::PRIM_TYPE modelPrim;
            GLenum modelPrimGl;
            std::shared_ptr<gfx::VertFormat> modelVertFormat;
            
            std::vector<uint32_t> morphIndices;
            std::vector<uint32_t> materialIndices;

            std::vector<std::shared_ptr<MaterialAndMeshPair>> matMeshes;
            // std::unordered_map<uint32_t, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialIdx;

            // Additional internal data...

            std::vector<std::string> morphNames;
            std::vector<std::shared_ptr<gfx::Material>> materials;
            std::unordered_map<std::string, std::shared_ptr<MaterialAndMeshPair>> matMeshesByMaterialId;

            /** Helper constructor. */
            PartModel(::zmd2::PartModelData *src);
            PartModel(std::string id);

            /** Reserve and return a new MaterialAndMeshPair for given material ID. */
            std::shared_ptr<MaterialAndMeshPair> reserve_matmesh(const std::string &materialId, std::shared_ptr<gfx::Material> &material);

            /** Find and return MaterialAndMeshPair for given material ID. */
            std::shared_ptr<MaterialAndMeshPair> find_matmesh_by_material_id(const std::string &materialId) const;

            /** Link material references */
            void link_materials(const std::vector<std::shared_ptr<Material>> &modelMaterials);

            void update_refs(Model *mdl);
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
            Bone(::zmd2::BoneData *src);

            /** Updates internal data from given bone table. */
            void update_refs(Model *mdl);
        };

        /** Embedded Material: Parameters. */
        struct MetaMaterialParams {
            ZMD2_CULL cull = ZMD2_CULL_NOCULLING;
            bool zwrite = true;
            bool ztest = true;
            ZMD2_ZFUNC zfunc = ZMD2_ZFUNC_LESS;
            bool alphatest = false;
            int alphatestRef = 0;
            bool baseTexfilter = false;
            bool baseTexrepeat = false;
            std::string chainMaterialNext = "";
            std::string shadowPassMaterial = "";
            bool shadowPassMaterialCopy = true;
            std::string shadowPassMaterialCopyShader = "";
            bool transparent = false;
            ZMD2_BM blendmodeSrc = ZMD2_BM_SRC_ALPHA;
            ZMD2_BM blendmodeDst = ZMD2_BM_INV_SRC_ALPHA;
            ZMD2_BM_EQ blendmodeEq = ZMD2_BM_EQ_ADD;

            std::string to_string() const;
        };

        /** Embedded Material: Uniforms. */
        struct MetaUniform {
            std::string name = "";
            std::string type = "";

            virtual std::string to_string() const;
        };

        struct MetaUniformSampler: public MetaUniform {
            std::string value = "";
            bool samplerTexfilter = false;
            bool samplerTexrepeat = false;

            std::string to_string() const;
        };
        struct MetaUniformCol: public MetaUniform {
            std::array<float, 4> value = { 0.0 };

            std::string to_string() const;
        };
        struct MetaUniformFloat: public MetaUniform {
            float value = 0.0;

            std::string to_string() const;
        };
        struct MetaUniformInt: public MetaUniform {
            float value = 0.0;

            std::string to_string() const;
        };
        struct MetaUniformVec: public MetaUniform {
            std::vector<float> value = { 0.0 };

            std::string to_string() const;
        };
        struct MetaUniformIvec: public MetaUniform {
            std::vector<int> value = { 0 };

            std::string to_string() const;
        };
        struct MetaUniformMat4: public MetaUniform {
            glm::mat4 value = glm::mat4(1.0);

            std::string to_string() const;
        };

        using MetaUniformAny = std::variant<
                MetaUniformSampler,
                MetaUniformCol,
                MetaUniformFloat,
                MetaUniformInt,
                MetaUniformVec,
                MetaUniformIvec,
                MetaUniformMat4
            >;

        /** ZMD2 Metadata: Embedded material info. */
        struct MetaEmbeddedMaterial {
            std::string name = "";
            std::string shader = "";
            std::string baseTexture = "";
            std::vector<std::string> textures = {};
            MetaMaterialParams params = {};
            std::vector<MetaUniformAny> uniforms = {};

            std::string to_string() const;
            /** Converts loaded data into actual material */
            gfx::Material to_material() const;
        };

        /** ZMD2 Metadata: Embedded texture info. */
        struct MetaEmbeddedTexture {
            std::string name = "";
            bool embedded = false;
            uint32_t width = 0;
            uint32_t height = 0;
            ZMD2_TEX_FORMAT format = ZMD2_TEX_FORMAT_RGBA8_UNORM;
            size_t blobOffset = 0;
            size_t blobSize = 0;

            std::string to_string() const;
            /** Converts loaded data into actual texture */
            gfx::Texture to_texture(std::span<const uint8_t> bytes) const;
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
            std::vector<MetaEmbeddedMaterial> embeddedMaterials;
            // (Metadata) Embedded textures
            std::vector<MetaEmbeddedTexture> embeddedTextures;

            // Internal cache
            std::vector<std::shared_ptr<gfx::Material>> linkedMaterials;
            std::unordered_map<std::string, std::shared_ptr<gfx::Texture>> linkedTexturesById;

            std::unordered_map<std::string, std::shared_ptr<Part>> partsById;
            std::unordered_map<std::string, std::shared_ptr<Bone>> bonesById;
        
        public:
            // Helper constructor.
            Model(::zmd2::Model *src);
            Model(std::string id);

            void update_refs_and_link_assets(const std::shared_ptr<gfx::Material> &fallbackMaterial = nullptr);

            void add_part(const std::shared_ptr<Part> &part);

            void add_bone(const std::shared_ptr<Bone> &bone);

            void link_material(const int idx, const std::shared_ptr<gfx::Material> &material);

            void link_texture(const std::string &textureId, const std::shared_ptr<gfx::Texture> &texture);

            /** Find and return part for given ID. */
            template <typename T>
            std::shared_ptr<T> find_part_by_id(const std::string &id) const;

            /** Find and return bone for given ID. */
            std::shared_ptr<Bone> find_bone_by_id(const std::string &id) const;

            /** Find and return linked material for given ID. */
            std::shared_ptr<gfx::Material> find_linked_material_by_id(const std::string &id) const;

            /** Find and return linked texture for given ID. */
            std::shared_ptr<gfx::Texture> find_linked_texture_by_id(const std::string &id) const;

            /** Returns span to (immutable) internal bones list that can be iterated and etc. */
            std::span<const std::shared_ptr<Bone>> all_bones() const;

            /** Returns span to (immutable) internal parts list that can be iterated and etc. */
            std::span<const std::shared_ptr<Part>> all_parts() const;

            /** Returns span to (immutable) internal material names list that can be iterated and etc. */
            std::span<const std::string> all_material_ids() const;

            /** Returns span to (immutable) internal morph names list that can be iterated and etc. */
            std::span<const std::string> all_morph_names() const;

            /** Returns span to (immutable) internal embedded materials list that can be iterated and etc. */
            std::span<const MetaEmbeddedMaterial> all_embedded_materials() const;

            /** Returns span to (immutable) internal embedded textures list that can be iterated and etc. */
            std::span<const MetaEmbeddedTexture> all_embedded_textures() const;

            /** Returns span to (immutable) internal array of bytes containing extra daa that can be iterated and etc. */
            std::span<const uint8_t> get_extra_bytes() const;

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

        // JSON deserialization implementations
        // https://json.nlohmann.me/features/arbitrary_types/

        void from_json(const json &src, MetaUniformAny &val);
        void from_json(const json &src, MetaMaterialParams &val);
        void from_json(const json &src, MetaEmbeddedMaterial &val);

        NLOHMANN_JSON_SERIALIZE_ENUM(ZMD2_CULL, {
            {ZMD2_CULL_NOCULLING,        "cull_noculling"},
            {ZMD2_CULL_CLOCKWISE,        "cull_clockwise"},
            {ZMD2_CULL_COUNTERCLOCKWISE, "cull_counterclockwise"},
        })
        NLOHMANN_JSON_SERIALIZE_ENUM(ZMD2_ZFUNC, {
            {ZMD2_ZFUNC_NEVER,           "cmpfunc_never"},
            {ZMD2_ZFUNC_LESS,            "cmpfunc_less"},
            {ZMD2_ZFUNC_EQUAL,           "cmpfunc_equal"},
            {ZMD2_ZFUNC_LESSEQUAL,       "cmpfunc_lessequal"},
            {ZMD2_ZFUNC_GREATER,         "cmpfunc_greater"},
            {ZMD2_ZFUNC_NOTEQUAL,        "cmpfunc_notequal"},
            {ZMD2_ZFUNC_GREATEREQUAL,    "cmpfunc_greaterequal"},
            {ZMD2_ZFUNC_ALWAYS,          "cmpfunc_always"},
        })
        NLOHMANN_JSON_SERIALIZE_ENUM(ZMD2_BM, {
            {ZMD2_BM_ZERO,           "bm_zero"},
            {ZMD2_BM_ONE,            "bm_one"},
            {ZMD2_BM_SRC_COL,        "bm_src_colour"},
            {ZMD2_BM_INV_SRC_COL,    "bm_inv_src_colour"},
            {ZMD2_BM_SRC_ALPHA,      "bm_src_alpha"},
            {ZMD2_BM_INV_SRC_ALPHA,  "bm_inv_src_alpha"},
            {ZMD2_BM_DST_COL,        "bm_dest_colour"},
            {ZMD2_BM_INV_DST_COL,    "bm_inv_dest_colour"},
            {ZMD2_BM_DST_ALPHA,      "bm_dest_alpha"},
            {ZMD2_BM_INV_DST_ALPHA,  "bm_inv_dest_alpha"},
            {ZMD2_BM_SRC_ALPHA_SAT,  "bm_src_alpha_sat"},
        })
        NLOHMANN_JSON_SERIALIZE_ENUM(ZMD2_BM_EQ, {
            {ZMD2_BM_EQ_ADD,                 "bm_eq_add"},
            {ZMD2_BM_EQ_SUBTRACT,            "bm_eq_subtract"},
            {ZMD2_BM_EQ_REVERSE_SUBTRACT,    "bm_eq_reverse_subtract"},
            {ZMD2_BM_EQ_MIN,                 "bm_eq_min"},
            {ZMD2_BM_EQ_MAX,                 "bm_eq_max"},
        })
        NLOHMANN_JSON_SERIALIZE_ENUM(ZMD2_TEX_FORMAT, {
            {ZMD2_TEX_FORMAT_RGBA8_UNORM, "surface_rgba8unorm"},
            {ZMD2_TEX_FORMAT_R8_UNORM, "surface_r8unorm"},
            {ZMD2_TEX_FORMAT_RG8_UNORM, "surface_rg8unorm"},
            {ZMD2_TEX_FORMAT_RGBA4_UNORM, "surface_rgba4unorm"},
            {ZMD2_TEX_FORMAT_RGBA16_FLOAT, "surface_rgba16float"},
            {ZMD2_TEX_FORMAT_R16_FLOAT, "surface_r16float"},
            {ZMD2_TEX_FORMAT_RGBA32_FLOAT, "surface_rgba32float"},
            {ZMD2_TEX_FORMAT_R32_FLOAT, "surface_r32float"},
        })

        // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MetaMaterialParams, cull, zwrite, ztest, zfunc, alphatest, alphatestRef, baseTexfilter, baseTexrepeat, chainMaterialNext, shadowPassMaterial, shadowPassMaterialCopy, shadowPassMaterialCopyShader, transparent, blendmodeSrc, blendmodeDst, blendmodeEq)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(MetaMaterialParams, cull, zwrite, ztest, zfunc, alphatest, alphatestRef, baseTexfilter, baseTexrepeat, chainMaterialNext, shadowPassMaterial, shadowPassMaterialCopy, shadowPassMaterialCopyShader, transparent, blendmodeSrc, blendmodeDst, blendmodeEq)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MetaUniform, name, type)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformSampler, MetaUniform, value, samplerTexfilter, samplerTexrepeat)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformCol, MetaUniform, value)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformFloat, MetaUniform, value)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformInt, MetaUniform, value)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformVec, MetaUniform, value)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformIvec, MetaUniform, value)
        NLOHMANN_DEFINE_DERIVED_TYPE_NON_INTRUSIVE(MetaUniformMat4, MetaUniform, value)
        // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MetaEmbeddedMaterial, name, shader, textures, params, uniforms)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(MetaEmbeddedMaterial, name, shader, textures, params, uniforms)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MetaEmbeddedTexture, name, embedded, width, height, format, blobOffset, blobSize)
    
        // For {fmt}
        std::string format_as(MetaEmbeddedTexture val);
        std::string format_as(MetaEmbeddedMaterial val);
        std::string format_as(MetaMaterialParams val);
        std::string format_as(MetaUniformSampler val);
        std::string format_as(MetaUniformCol val);
        std::string format_as(MetaUniformFloat val);
        std::string format_as(MetaUniformInt val);
        std::string format_as(MetaUniformVec val);
        std::string format_as(MetaUniformIvec val);
        std::string format_as(MetaUniformMat4 val);
    }
}

// Third party (GLM types) conversion definitions

NLOHMANN_JSON_NAMESPACE_BEGIN

template <>
struct adl_serializer<glm::mat4> {
    static void from_json(const json &src, glm::mat4 &val) {
        auto uniformsSz = src.size();
        
        assert(uniformsSz == 16 && "MAT4 UNIFORMS MUST BE 16 VALUED ARRAY");
        
        auto values = src.get<std::array<float, 16>>();

        val = glm::make_mat4(values.data());
    };

    static void to_json(json &src, const glm::mat4 &val) {
        std::array<float, 16> flattened;

        std::copy(glm::value_ptr(val), glm::value_ptr(val) + 16, flattened.begin());
        src = json { flattened };
    };
};

NLOHMANN_JSON_NAMESPACE_END

#endif
