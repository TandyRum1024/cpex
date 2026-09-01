/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#ifndef __ZCL_GUARD
#define __ZCL_GUARD

#include <string>
#include <fstream>

// EXTERNAL LIBRARIES //
#include <spdlog/spdlog.h>

namespace zcl {
    namespace file {
        /** Reads the entire contents of given file intro a string and returns it. */
        std::string read_file_to_string(std::string filePath);
    }

    /** Returns a logger. */
    std::shared_ptr<spdlog::logger> logger(const std::string &name);
}
#endif