/**
 * gfx::vert - Vertex data holding classes.
 * ZIK@MMXXVI
 */

#include <gfx/vert.hpp>

using namespace gfx;

VertPosUv::VertPosUv(glm::vec3 pos, glm::vec2 uv):
    pos(pos),
    uv(uv)
    {}
