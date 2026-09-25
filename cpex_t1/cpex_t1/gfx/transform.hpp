/**
 * gfx::tf - Transform data
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_TF_GUARD
#define __CPEX_GFX_TF_GUARD

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    /** Transform data. */
    struct Transform {
        glm::vec3 pos;
        glm::quat rot;
        glm::vec3 scale;

        Transform();
        Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale);
        Transform(glm::vec3 pos, glm::vec3 eulerAnglesRad, glm::vec3 scale);

        /** Convert this transform to 4x4 transform matrix. */
        glm::mat4 to_mat4() const;
    };
}

#endif