/**
 * zmd2 - `.zmd2` file format.
 * ZIK@MMXXVI
 */

#ifndef __ZMD2_GUARD
#define __ZMD2_GUARD

#include <string>
#include <memory>

#include <zmd2/types.hpp>

namespace zmd2 {
    std::shared_ptr<Model> load_model_from(const std::string &id, std::istream &in, std::streampos begin = -1, std::streampos end = -1);
}
#endif