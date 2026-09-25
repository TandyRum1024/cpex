/**
 * gfx::math - Math helpers
 * ZIK@MMXXVI
 */

#include <gfx/math.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace gfx;

glm::mat4 gfx::mat4_transform(const glm::vec3 pos, const glm::vec3 eulerAnglesRad, const glm::vec3 scale) {
    return glm::scale(
            glm::rotate(
                glm::rotate(
                    glm::rotate(
                        glm::translate(
                            glm::mat4(1.0),
                            pos
                        ),
                        eulerAnglesRad.x,
                        glm::vec3(1.0, 0.0, 0.0)
                    ),
                    eulerAnglesRad.y,
                    glm::vec3(0.0, 1.0, 0.0)
                ),
                eulerAnglesRad.z,
                glm::vec3(0.0, 0.0, 1.0)
            ),
            scale
        );
}

glm::mat4 gfx::mat4_transform(const glm::vec3 pos, const glm::quat quat, const glm::vec3 scale) {
    return glm::scale(
                glm::translate(glm::mat4(1.0), pos) * glm::mat4_cast(quat),
                scale
            );
}