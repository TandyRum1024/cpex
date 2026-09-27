/**
 * zcl::zlib - Zlib decompression stream wrappers and helper.
 * ZIK@MMXXVI
 */

#ifndef __ZCL_ZLIB_GUARD
#define __ZCL_ZLIB_GUARD

// EXTERNAL LIBRARIES //
// ----------------------------
// zlib
#include <zlib.h>
// ----------------------------
// EXTERNAL LIBRARIES //

namespace zcl {
    namespace zlib {
        // https://stackoverflow.com/questions/14086417/how-to-write-custom-input-stream-in-c
        // https://gist.github.com/andik/c55bb4bc49b54c424935
        // https://en.cppreference.com/cpp/io/basic_streambuf/underflow
        /**
         * Stupid `std::streambuf` wrapper for zlibs stream based `inflate()` function.
         * Super lazy implementation of inflation routine. Until I make a proper stream wrapper...
         * Until I learn more about multithreading in C++, this is NOT THREAD SAFE!!
         **/
        class inflated_streambuf: public std::streambuf {
            const static int CHUNK_BYTES = 1024;

            std::streambuf* src;
            std::streampos begin;
            std::streampos end;

            z_stream stream;

            bool isEnd;
            bool isInflateDone;
            bool isSuccess;
            std::array<uint8_t, CHUNK_BYTES> chunkDeflated;
            std::array<uint8_t, CHUNK_BYTES> chunkInflated;
            size_t readSzTotal;
            size_t readSzLeft;
            size_t readPosBegin;
            size_t readPosEnd;
            size_t readChunksTotal;

        public:
            inflated_streambuf(std::streambuf* src, std::streampos begin, std::streampos end);
            ~inflated_streambuf();

            inflated_streambuf(inflated_streambuf &&other);
            inflated_streambuf& operator=(inflated_streambuf &&other);

            inflated_streambuf(const inflated_streambuf &other) = delete;
            inflated_streambuf& operator=(const inflated_streambuf &other) = delete;
            
            void read_src_to_chunk();
            unsigned int inflate_from_chunk();
            
        protected:
            std::streambuf::int_type overflow(std::streambuf::int_type ch) override;
            std::streambuf::int_type underflow() override;

            std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which) override;
            std::streambuf::pos_type seekoff(std::streambuf::off_type pos, std::ios_base::seekdir dir, std::ios_base::openmode which) override;
            int sync() override;
        };

        /** Inflates given compressed stream with zlib, converts it into output stream. */
        int inflate_stream_to(std::istream &in, std::stringstream &out, std::streampos begin, std::streampos end);
    }
}
#endif