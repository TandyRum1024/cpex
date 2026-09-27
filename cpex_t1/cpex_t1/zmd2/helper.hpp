/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_HELPER_GUARD
#define __ZMD2_HELPER_GUARD

#include <zmd2/types.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>

namespace zmd2 {
    Bbox zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes);
}
#endif