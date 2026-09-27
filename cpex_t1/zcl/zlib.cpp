/**
 * zcl::zlib - Zlib decompression stream wrappers and helper.
 * ZIK@MMXXVI
 */

#include <zcl/zcl.hpp>
#include <zcl/zlib.hpp>

using namespace zcl;

zlib::inflated_streambuf::inflated_streambuf(std::streambuf* src, std::streampos begin, std::streampos end):
    src(src),
    begin(begin),
    end(end),
    stream(),
    isEnd(false),
    isInflateDone(true),
    isSuccess(false),
    chunkDeflated(),
    chunkInflated(),
    readSzTotal(0),
    readSzLeft(end - begin),
    readPosBegin(begin),
    readPosEnd(end),
    readChunksTotal(0) {
        // Initialize stream for zlib
        // https://zlib.net/zlib_how.html
        stream = {
            .next_in = Z_NULL,
            .avail_in = 0,
            .zalloc = Z_NULL,
            .zfree = Z_NULL,
            .opaque = Z_NULL
        };
        auto res = inflateInit(&stream);

        if (res != Z_OK) {
            zcl::logger("ZLIB")->error("`inflateInit()` FAILED!");
        }
        
        zcl::logger("ZLIB")->info("(@{}: BEGINNING INFLATION WITH STATE: {}, STREAM STATE: {}!!)", fmt::ptr(this), fmt::ptr(&stream), fmt::ptr(stream.state));

        src->pubseekpos(begin, std::ios_base::in);
        src->pubseekpos(begin, std::ios_base::out);
        setg(NULL, NULL, NULL);
    }
zlib::inflated_streambuf::~inflated_streambuf() {
    zcl::logger("ZLIB")->info("(@{}: ENDING INFLATION WITH STATE {}!!)", fmt::ptr(this), fmt::ptr(&stream));
    inflateEnd(&stream);
}

zlib::inflated_streambuf::inflated_streambuf(inflated_streambuf &&other):
    std::streambuf::basic_streambuf(std::move(other)),
    src(std::exchange(other.src, nullptr)),
    begin(std::exchange(other.begin, 0)),
    end(std::exchange(other.end, 0)),
    // stream(std::exchange(other.stream, {})),
    isEnd(std::exchange(other.isEnd, false)),
    isInflateDone(std::exchange(other.isInflateDone, true)),
    isSuccess(std::exchange(other.isSuccess, false)),
    // chunkDeflated(std::exchange(other.chunkDeflated, {})),
    // chunkInflated(std::exchange(other.chunkInflated, {})),
    chunkDeflated({}),
    chunkInflated({}),
    readSzTotal(std::exchange(other.readSzTotal, 0)),
    readSzLeft(std::exchange(other.readSzLeft, 0)),
    readPosBegin(std::exchange(other.readPosBegin, 0)),
    readPosEnd(std::exchange(other.readPosEnd, 0)),
    readChunksTotal(std::exchange(other.readChunksTotal, 0)) {
        // Prevent rvalue streambuf destroying moved streams and likes
        inflateCopy(&stream, &other.stream);
        // other.src = nullptr;
        // other.stream = z_stream {};

        zcl::logger("ZLIB")->info("(@{}: MOVE FROM @{}, STATE: {}, STREAM STATE: {} VS {})", fmt::ptr(this), fmt::ptr(&other), fmt::ptr(&stream), fmt::ptr(stream.state), fmt::ptr(other.stream.state));
        setg(NULL, NULL, NULL);
    }

zlib::inflated_streambuf& zlib::inflated_streambuf::operator=(inflated_streambuf &&other) {
    zcl::logger("ZLIB")->info("(@{}: MOVE ASSIGN TO @{})", fmt::ptr(&other), fmt::ptr(this));

    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }

    src = std::exchange(other.src, nullptr);
    std::swap(begin, other.begin);
    std::swap(end, other.end);
    // stream = std::exchange(other.stream, {});
    // std::swap(stream, other.stream);
    std::swap(isEnd, other.isEnd);
    std::swap(isInflateDone, other.isInflateDone);
    std::swap(isSuccess, other.isSuccess);
    std::swap(chunkDeflated, other.chunkDeflated);
    std::swap(chunkInflated, other.chunkInflated);
    std::swap(readSzTotal, other.readSzTotal);
    std::swap(readSzLeft, other.readSzLeft);
    std::swap(readPosBegin, other.readPosBegin);
    std::swap(readPosEnd, other.readPosEnd);
    std::swap(readChunksTotal, other.readChunksTotal);
    
    // Prevent rvalue streambuf destroying moved streams and likes
    inflateCopy(&stream, &other.stream);
    // other.stream = z_stream {};
    std::streambuf::operator=(std::move(other));

    setg(NULL, NULL, NULL);
    return *this;
}

void zlib::inflated_streambuf::read_src_to_chunk() {
    auto chunkSz = std::min((size_t) CHUNK_BYTES, readSzLeft);

    if (chunkSz <= 0) {
        isEnd = true;
        return;
    }

    zcl::logger("ZLIB")->info("READING {}/{}, {} BYTES", readPosBegin + readSzTotal, readPosEnd, chunkSz);

    // Read a chunk from stream
    // in >> chunkIn;
    auto readSz = src->sgetn(reinterpret_cast<char*>(chunkDeflated.data()), chunkSz);
    
    if (readSz <= 0) {
        zcl::logger("ZLIB")->info("END READ {}/{}", 0 + begin + readSzTotal, 0 + end);
        isEnd = true;
        return;
    }
    
    readSzTotal += readSz;
    readSzLeft -= readSz;
    readChunksTotal++;

    zcl::logger("ZLIB")->info("READ {}/{} (+ {})", readPosBegin + readSzTotal, readPosEnd, readSz);
    // auto chunkStr = std::string(reinterpret_cast<char*>(chunkDeflated.data()), chunkDeflated.size());
    // zcl::logger("ZLIB")->info("{}", chunkStr);

    // Ready for the next round of `inflate()` calls
    stream.next_in = chunkDeflated.data();
    stream.avail_in = readSz;
    isInflateDone = false;
}

unsigned int zlib::inflated_streambuf::inflate_from_chunk() {
    stream.next_out = chunkInflated.data();
    stream.avail_out = CHUNK_BYTES;

    auto res = inflate(&stream, Z_NO_FLUSH);

    zcl::logger("ZLIB")->info("\tINFLATED {}/{} (STREAM STATE = {}, {})", CHUNK_BYTES - stream.avail_out, CHUNK_BYTES, fmt::ptr(stream.state), (stream.msg) ? stream.msg : "NO ERROR");

    switch (res) {
        case Z_MEM_ERROR:
            throw std::runtime_error("Out of memory!");
            break;
        case Z_NEED_DICT:
            throw std::runtime_error("Compression data incomplete / invalid! (DICT NEEDED)");
            break;
        case Z_DATA_ERROR:
            throw std::runtime_error("Compression data incomplete / invalid!");
            break;
        case Z_STREAM_ERROR: // not normally possible
            throw std::runtime_error("UNKNOWN ERROR!");
            break;
        
        case Z_STREAM_END:
            isSuccess = true;
            break;
    }

    auto chunkStr = std::string(reinterpret_cast<char*>(chunkInflated.data()), CHUNK_BYTES - stream.avail_out);
    zcl::logger("ZLIB")->info("\tCHUNK: {}", chunkStr);
    
    isInflateDone = (stream.avail_out != 0 || isSuccess);
    return CHUNK_BYTES - stream.avail_out;
}

std::streambuf::int_type zlib::inflated_streambuf::overflow(std::streambuf::int_type ch) {
    zcl::logger("ZLIB")->info("OVERFLOW, {}/{}, {}", readPosBegin + readSzTotal, readPosEnd, ch);
    return std::streambuf::overflow(ch);
}

std::streambuf::int_type zlib::inflated_streambuf::underflow() {
    try {
        zcl::logger("ZLIB")->info("UNDERFLOW, {}/{}", readPosBegin + readSzTotal, readPosEnd);

        if (gptr() == egptr()) {
            // zcl::logger("ZLIB")->info("\t(NEEDS MORE STREAMING)");

            if (!isEnd && isInflateDone) {
                read_src_to_chunk();
            }

            if (!isEnd) {
                auto writeSz = inflate_from_chunk();
                auto chunkPtr = reinterpret_cast<char*>(chunkInflated.data());

                setg(chunkPtr, chunkPtr, chunkPtr + writeSz);
                // zcl::logger("ZLIB")->info("\tSETG {0}-{1} ({2} BYTES) '{3} ( {3:c} )'", fmt::ptr(chunkPtr), fmt::ptr(chunkPtr + writeSz), writeSz, *gptr());
            }
        }

        // zcl::logger("ZLIB")->info("\tRET {0:d} ( {0:c} ), COMP: {1}", gptr() ? (*gptr()) : '?', gptr() == egptr());
        return (gptr() == egptr())
                    ? traits_type::eof()
                    : traits_type::to_int_type(*gptr());
    }
    catch (std::exception e) {
        zcl::logger("ZLIB")->error(fmt::format("ZLIB ERROR: {}", e.what()));
        throw e;
    }
    // (until I drag my lazy body to write a proper RAII based resource handling, a hack should suffice...)
    catch (...) {
        zcl::logger("ZLIB")->error("ZLIB ERROR DETECTED!!");
        throw std::current_exception();
    }
}

std::streambuf::pos_type zlib::inflated_streambuf::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which) {
    zcl::logger("ZLIB")->info("(@{}: SEEK TO {})", fmt::ptr(this), 0 + pos);
    readSzLeft = readPosEnd - pos;
    return src->pubseekpos(pos, which);
}

std::streambuf::pos_type zlib::inflated_streambuf::seekoff(std::streambuf::off_type pos, std::ios_base::seekdir dir, std::ios_base::openmode which) {
    auto res = src->pubseekoff(pos, dir, which);

    zcl::logger("ZLIB")->info("(@{}: SEEK OFF TO {})", fmt::ptr(this), 0 + res);
    readSzLeft = readPosEnd - res;
    return res;
}

int zlib::inflated_streambuf::sync() {
    zcl::logger("ZLIB")->info("(@{}: SYNC)", fmt::ptr(this));
    return src->pubsync();
}