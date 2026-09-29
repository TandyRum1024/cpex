/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_HELPER_GUARD
#define __ZMD2_HELPER_GUARD

#include <vector>
#include <map>
#include <memory>

#include <zmd2/types.hpp>

// LIBRARIES //
#include <gfx/vert.hpp>
#include <gfx/transform.hpp>
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>

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

    const static std::unordered_map<std::string, ZMD2_MODEL> ZMD2_MODEL_FROM_NAME_TBL = {
        { "mesh", ZMD2_MODEL_MESH },
        { "mesh.morph.skinned", ZMD2_MODEL_MESH_MORPH_SKINNED },
        { "wire.morph.skinned", ZMD2_MODEL_WIRE_MORPH_SKINNED },
    };
    const static std::unordered_map<ZMD2_MODEL, std::shared_ptr<gfx::VertFormat>> ZMD2_MODEL_TO_FORMAT_TBL = {
        { ZMD2_MODEL_MESH, ZMD2_VERT_FORMAT_MESH },
        { ZMD2_MODEL_MESH_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
        { ZMD2_MODEL_WIRE_MORPH_SKINNED, ZMD2_VERT_FORMAT_MESH_MORPH_SKINNED },
    };
    const static std::unordered_map<ZMD2_MODEL, GLenum> ZMD2_MODEL_TO_PRIM_TBL = {
        { ZMD2_MODEL_MESH, GL_TRIANGLES },
        { ZMD2_MODEL_MESH_MORPH_SKINNED, GL_TRIANGLES },
        { ZMD2_MODEL_WIRE_MORPH_SKINNED, GL_LINES },
    };

    void zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes, Bbox &out);
    void zmd2_load_tf_from_buffer(zcl::stream::byteistream &bytes, gfx::Transform &out);
    
    void zmd2_load_bone_from_buffer(zcl::stream::byteistream &bytes, Bone &out, const std::string &id);
    std::shared_ptr<Part> zmd2_load_part_from_buffer(zcl::stream::byteistream &bytes, const std::string &id);

    // Template functions are defined here as inclusion model is required for this

    template <typename V>
    void zmd2_load_verts_from_buffer(zcl::stream::byteistream &bytes, std::vector<V> &out, size_t numBytes, size_t numVerts) {
        // Sanity check
        if (numBytes != sizeof(V) * numVerts) {
            throw std::runtime_error(fmt::format("Buffer size vs actual size mismatch! (expected: {} vs numBytes: {})", sizeof(V) * numVerts, numBytes));
        }

        // Somewhat hacky & dirty but since vectors are guranteed to be continuous this may be fine enough...
        out.resize(numVerts);
        bytes.read(out.data(), numBytes);
    }

    template <typename T>
    void zmd2_load_vector(zcl::stream::byteistream &bytes, std::vector<T> &out) {
        // auto numBytes = out.size() * sizeof(T);
        // bytes.read(reinterpret_cast<char*>(out.data()), numBytes);

        for (auto i=0; i<out.size(); i++) {
            bytes >> out[i];
        }
    }
}
#endif