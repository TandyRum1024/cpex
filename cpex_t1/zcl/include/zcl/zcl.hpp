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

    namespace str {
        /** Converts `std::array` to string. */
        template <typename T, int N>
        inline std::string to_str(std::array<T, N> arr, const char* delim = ", ") {
            //return std::string(arr.begin(), arr.end());
            std::string res;

            for (auto it=arr.begin(); it!=arr.end(); it++) {
                res += std::to_string(*it);

                if ((it + 1) != arr.end()) {
                    res += delim;
                }
            }

            return res;
        }
    }

    /** Returns a logger. */
    std::shared_ptr<spdlog::logger> logger(const std::string &name);
}
#endif