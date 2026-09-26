/**
 * gfx::tf - Transform data
 * ZIK@MMXXVI
 */

#include <gfx/transform.hpp>
#include <gfx/math.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/gtc/matrix_transform.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

Transform::Transform():
    Transform(glm::vec3(0.0), glm::quat(glm::vec3(0.0)), glm::vec3(1.0)) {}

Transform::Transform(glm::vec3 pos, glm::vec3 eulerAnglesRad, glm::vec3 scale):
    Transform(pos, glm::quat(eulerAnglesRad), scale) {}

Transform::Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale):
    pos(pos),
    rot(rot),
    scale(scale) {}


glm::mat4 Transform::to_mat4() const {
    return mat4_transform(pos, rot, scale);
}