/**
 * gfx::vert - Vertex data holding classes.
 * ZIK@MMXXVI
 */

#include <gfx/vert.hpp>

using namespace gfx;

const std::shared_ptr<gfx::VertFormat> VertPosUv::format = std::make_shared<gfx::VertFormat>(
    gfx::VertFormat({
        gfx::VertAttribute(0, 3, GL_FLOAT, sizeof(float), 5, 0), // POS
        gfx::VertAttribute(1, 2, GL_FLOAT, sizeof(float), 5, 3), // UV
    })
);

VertPosUv::VertPosUv(glm::vec3 pos, glm::vec2 uv):
    pos(pos),
    uv(uv) {}
