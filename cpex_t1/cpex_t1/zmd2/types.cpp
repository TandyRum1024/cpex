/**
 * zmd2::types - Types used in `.zmd2` model.
 * ZIK@MMXXVI
 */

#include <zmd2/types.hpp>

using namespace zmd2;

void Bbox::merge_from(Bbox &other) {
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}