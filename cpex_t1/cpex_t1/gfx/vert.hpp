/**
 * gfx::vert - Vertex data holding classes.
 * ZIK@MMXXVI
 */

#ifndef __CPEX_GFX_VERT_GUARD
#define __CPEX_GFX_VERT_GUARD

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace gfx {
    #pragma pack(push, 1)
    /** Example struct for vertices data, containing xyz positions and uv coordinates. */
    struct VertPosUv {
        glm::vec3 pos;
        glm::vec2 uv;

    public:
        VertPosUv(glm::vec3 pos, glm::vec2 uv);
    };
    #pragma pack(pop)
}
#endif