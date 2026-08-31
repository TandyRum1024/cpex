#ifndef __ZCL_GUARD
#define __ZCL_GUARD
#include <string>
#include <fstream>

namespace zcl {
    namespace file {
        /** Reads the entire contents of given file intro a string and returns it. */
        std::string read_file_to_string(std::string filePath);
    }
}
#endif