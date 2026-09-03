/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#ifndef __ZCL_GUARD
#define __ZCL_GUARD

#include <string>
#include <fstream>
#include <filesystem>

// EXTERNAL LIBRARIES //
// ----------------------------
#include <spdlog/spdlog.h>
// OS
// https://github.com/cpredef/predef/blob/master/OperatingSystems.md
#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
// TODO
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(Macintosh) || defined(macintosh)
#include <mach-o/dyld.h>
#endif
// ----------------------------
// EXTERNAL LIBRARIES //

namespace zcl {
    namespace file {
        /** Reads the entire contents of given file intro a string and returns it. */
        std::string read_file_to_string(std::filesystem::path filePath);

        /** Returns path of executable. Useful for loading assets in relative path. */
        std::filesystem::path get_exec_path();
    }

    /** Returns a logger. */
    std::shared_ptr<spdlog::logger> logger(const std::string &name);
}
#endif