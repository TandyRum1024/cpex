/**
 * zcl::stream - Stream wrappers and helpers.
 * ZIK@MMXXVI
 */

#ifndef __ZCL_STREAM_GUARD
#define __ZCL_STREAM_GUARD

#include <istream>
#include <streambuf>

namespace zcl {
    namespace stream {
        /** `std::istream` wrapper for easier extraction of unformatted data. */
        class byteistream: public std::istream {
        public:
            byteistream(std::streambuf *sbuff);

            /** Fetches value sans formatting & whitespace skipping. Internally uses `istream.read()`! */
            template <typename T>
            byteistream& operator>>(T &value);

            /** Fetches string until null terminator. Internally uses `std::getline()`! */
            template <>
            byteistream& operator>>(std::string &value);
        };

        // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

        template <typename T>
        stream::byteistream& byteistream::operator>>(T &value) {
            // zcl::logger("ZCL")->info("[BYTEISTREAM] READING {} BYTES", sizeof(T));
            this->read(reinterpret_cast<char*>(&value), sizeof(T));

            return (*this);
        }
        
        // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //
    }
}
#endif