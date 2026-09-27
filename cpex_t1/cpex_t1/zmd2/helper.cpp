/**
 * zmd2::helper - Helper functions to be used in model loading etc.
 * ZIK@MMXXVI
 */

#include <zmd2/helper.hpp>
#include <zmd2/types.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

using namespace zmd2;

Bbox zmd2::zmd2_load_bbox_from_buffer(zcl::stream::byteistream &bytes) {
    float  minX = 0, minY = 0, minZ = 0,
            maxX = 0, maxY = 0, maxZ = 0;

    bytes >> minX >> minY >> minZ >> maxX >> maxY >> maxZ;
    return Bbox { glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ) };
}