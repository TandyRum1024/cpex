/**
 * gfx::math - Math helpers
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_MATH_GUARD
#define __CPEX_GFX_MATH_GUARD

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/glm.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Builds 4x4 transform matrix from given transform values. */
    glm::mat4 mat4_transform(const glm::vec3 pos, const glm::vec3 eulerAnglesRad, const glm::vec3 scale);
    /** Builds 4x4 transform matrix from given transform values. */
    glm::mat4 mat4_transform(const glm::vec3 pos, const glm::quat quat, const glm::vec3 scale);
}

#endif