/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_HELPER_GUARD
#define __ZMD2_HELPER_GUARD

#include <vector>
#include <memory>

#include <zmd2/types.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>

namespace zmd2 {
    void zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes, BboxData &out);
    void zmd2_load_tf_from_buffer(zcl::stream::byteistream &bytes, TfData &out);
    void zmd2_load_bone_from_buffer(zcl::stream::byteistream &bytes, std::shared_ptr<BoneData> &out, const std::string &id);
    void zmd2_load_part_from_buffer(zcl::stream::byteistream &bytes, std::shared_ptr<PartData> &out, const std::string &id);

    // Template functions are defined here as inclusion model is required for this

    template <typename V>
    void zmd2_load_verts_from_buffer(zcl::stream::byteistream &bytes, std::vector<V> &out) {
        // (TODO: check if V contains a legit vertex data)
        
        // Somewhat hacky & dirty but since vectors are guranteed to be continuous this may be fine enough...
        // https://herbsutter.com/2008/04/07/cringe-not-vectors-are-guaranteed-to-be-contiguous/
        auto numBytes = out.size() * sizeof(V);
        bytes.read(reinterpret_cast<char*>(out.data()), numBytes);
    }

    template <typename T>
    void zmd2_load_vector(zcl::stream::byteistream &bytes, std::vector<T> &out) {
        // Directly reading to data() field somewhat risky in this context since T could be std::string etc
        // auto numBytes = out.size() * sizeof(T);
        // bytes.read(reinterpret_cast<char*>(out.data()), numBytes);

        for (auto i=0; i<out.size(); i++) {
            bytes >> out[i];
        }
    }

    
}
#endif