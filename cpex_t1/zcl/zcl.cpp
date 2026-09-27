/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

// LIBRARY
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// OS
// https://github.com/cpredef/predef/blob/master/OperatingSystems.md
#if defined(_WIN32)
    #define NOMINMAX
    #include <Windows.h>
#elif defined(__linux__)
    // TODO
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(Macintosh) || defined(macintosh)
    #include <mach-o/dyld.h>
#endif

// (for stack trace)
#ifdef _MSC_VER
    #include <DbgHelp.h>
#else
    #include <execinfo.h>
#endif

// zlib
#include <zlib.h>
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zcl;

stream::byteistream::byteistream(std::streambuf *buff):
    std::istream(buff) {}

std::string io::read_file_to_string(std::filesystem::path filePath) {
    std::string contentStr;
    auto file = std::ifstream(filePath);
    
    if (file.is_open()) {
        std::streampos fileSz;

        file.seekg(0, std::ios::end);
        fileSz = file.tellg();
        contentStr.resize(fileSz);
        file.seekg(0, std::ios::beg);

        file.read(&contentStr[0], fileSz);
        // std::cout << "(SIZE: " << fileSz << ")\n" << contentStr;
    }
    else {
        throw std::runtime_error(std::string("read_file_to_string(): Could not open file `") + filePath.string() + "`");
    }

    return contentStr;
}

std::filesystem::path io::get_exec_path() {
    // https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
    std::filesystem::path path;

    #if defined(_WIN32)
        // Windows
        // https://learn.microsoft.com/en-gb/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea?redirectedfrom=MSDN
        // TODO: Windows might lie and MATH_PATH may not be sufficient? For now I'm using length that are long enough
        static const unsigned int WIN_PATH_LENGTH_MAX = 2048; // MAX_PATH
        wchar_t winPath[WIN_PATH_LENGTH_MAX] = {0};
        
        DWORD winPathLen = GetModuleFileNameW(NULL, winPath, WIN_PATH_LENGTH_MAX);
        path = std::filesystem::path(winPath);
    #elif defined(__linux__)
        // Linux
        // https://stackoverflow.com/questions/1528298/get-path-of-executable
        path = std::filesystem::canonical("/proc/self/exe");
    #elif (defined(__APPLE__) && defined(__MACH__)) || defined(Macintosh) || defined(macintosh)
        // Mac
        // (UNTESTED!)
        static const unsigned int MAC_PATH_LENGTH_MAX = 2048; // PATH_MAX
        uint32_t macPathLen = MAC_PATH_LENGTH_MAX;
        wchar_t macPath[MAC_PATH_LENGTH_MAX];

        if (!_NSGetExecutablePath(macPath, &macPathLen)) {
            path = std::filesystem::path(macPath);
        }
        else {
            throw std::runtime_error("get_exec_path(): `_NSGetExecutablePath()` Failed!");
        }
    #else
        throw std::exception("get_exec_path(): Unsupported OS!");
    #endif

    return path;
}

std::shared_ptr<spdlog::logger> zcl::logger(const std::string &name) {
    auto logger = spdlog::get(name);

    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
        // _logger->set_level(spdlog::level::debug);
    }
    return logger;
}

// zcl::trace

trace::StopwatchRepository& trace::StopwatchRepository::get_instance() {
    static StopwatchRepository inst = StopwatchRepository();
    return inst;
}

std::weak_ptr<trace::StopwatchSplit> trace::StopwatchRepository::get_scope_parent_split() const {
    // if (auto s = splitCurrent.lock()) {
    //     logger("SPLIT")->info("CURRENT PARENT IS {}", s->get_id());
    // }
    return splitCurrent;
}

void trace::StopwatchRepository::set_scope_parent_split(std::weak_ptr<trace::StopwatchSplit> split) {
    // if (id.empty()) {
    //     reset_scope_parent_split();
    // }
    // else {
    //     auto res = get_split(id);
    //     splitCurrent = res;
    // }

    // if (auto s = split.lock()) {
    //     logger("SPLIT")->info("+ SET PARENT {}", s->get_id(), fmt::ptr(this));
    // }
    splitCurrent = split;
}

void trace::StopwatchRepository::reset_scope_parent_split() {
    // if (auto s = splitCurrent.lock()) {
    //     logger("SPLIT")->info("- RESET PARENT FROM {}", s->get_id());
    // }
    splitCurrent.reset();
}

std::shared_ptr<trace::StopwatchSplit> trace::StopwatchRepository::get_split(const std::string &id) {
    auto res = std::find_if(
        splits.begin(),
        splits.end(),
        [&] (std::shared_ptr<StopwatchSplit> split) {
            return id == split->get_id();
        }
    );

    if (res == splits.end()) {
        // logger("SPLIT")->info("NO SPLIT FOUND, MAKING NEW SPLIT {}", id);

        // Create new instance and return
        // auto split = StopwatchSplit();
        auto splitPtr = std::make_shared<StopwatchSplit>(StopwatchSplit(id));

        splits.push_back(splitPtr);
        return splitPtr;
    }
    else {
        // Return query result
        return *res;
    }
}

std::shared_ptr<trace::StopwatchSplitHelper> trace::StopwatchRepository::begin_split(const std::string &id) {
    auto splitPtr = get_split(id);
    auto helper = std::make_shared<StopwatchSplitHelper>(StopwatchSplitHelper(splitPtr));

    return helper;
}

std::vector<std::shared_ptr<trace::StopwatchSplitNode>> trace::StopwatchRepository::get_all_splits_and_childs() const {
    auto roots = std::vector<std::shared_ptr<StopwatchSplitNode>>();
    auto nodesById = std::map<std::string, std::shared_ptr<StopwatchSplitNode>>();

    // For all splits, create map of children splits indexed by parents id
    // And also prepare list of "roots"
    for (auto &&split: splits) {
        auto splitId = split->get_id();
        auto splitNode = nodesById[splitId];

        // Initialize node
        if (!splitNode) {
            splitNode = std::make_shared<StopwatchSplitNode>(StopwatchSplitNode(splitId));
            nodesById[splitId] = splitNode;
            splitNode->id = splitId;
            splitNode->duration = split->calc_duration();
        }

        if (auto parent = split->get_parent().lock()) {
            // Has a parent!
            auto parentId = parent->get_id();
            auto parentNode = nodesById[parentId];

            // Initialize parent if needed
            if (!parentNode) {
                parentNode = std::make_shared<StopwatchSplitNode>(StopwatchSplitNode(parentId));
                nodesById[splitId] = parentNode;
                parentNode->id = parentId;
                parentNode->duration = split->calc_duration();
            }

            parentNode->children.push_back(splitNode);
        }
        else {
            // No parent. Might be the "root"
            roots.push_back(splitNode);
        }
    }

    return roots;
}

trace::StopwatchSplitNode::StopwatchSplitNode(): StopwatchSplitNode("") {}
trace::StopwatchSplitNode::StopwatchSplitNode(const std::string &id): id(id) {}

trace::StopwatchSplitNode::operator std::string() const {
    return fmt::format("[{}]: {:.4}ms", id, duration.count());
}

trace::StopwatchSplit::StopwatchSplit(): StopwatchSplit("") {}
trace::StopwatchSplit::StopwatchSplit(const std::string &id): id(id) {
    // logger("SPLIT")->info("NEW SPLIT {} {}", id, fmt::ptr(this));
}

std::weak_ptr<trace::StopwatchSplit> trace::StopwatchSplit::get_parent() const {
    return parentSplit;
}

void trace::StopwatchSplit::set_parent(std::weak_ptr<StopwatchSplit> split) {
    parentSplit = split;
}

std::string trace::StopwatchSplit::get_id() {
    return id;
}

void trace::StopwatchSplit::begin_sprint() {
    timeEnd = std::chrono::steady_clock::now();
    timeBegin = std::chrono::steady_clock::now();
}

void trace::StopwatchSplit::end_sprint() {
    timeEnd = std::chrono::steady_clock::now();
    timeDelta = timeEnd - timeBegin;
}

std::chrono::duration<double, std::milli> trace::StopwatchSplit::calc_duration() const {
    auto dst = (timeEnd < timeBegin) ? (std::chrono::steady_clock::now() - timeBegin) : timeDelta;
    // logger("tr")->info("STOPWATCH {} DURATION: {}", name, timeBegin.time_since_epoch().count());
    return dst;
}

// Cast to `std::string`.
// https://en.cppreference.com/cpp/language/cast_operator
trace::StopwatchSplit::operator std::string() const {
    auto duration = calc_duration();
    return fmt::format("[{}]: {:.4}ms", id, duration.count());
}

trace::StopwatchSplitHelper::StopwatchSplitHelper(StopwatchSplitHelper&& other):
    split(std::move(other.split)) {
        other.split = std::weak_ptr<StopwatchSplit>();
    }

trace::StopwatchSplitHelper::StopwatchSplitHelper(std::weak_ptr<StopwatchSplit> split):
    split(split)
    {
    auto& repo = StopwatchRepository::get_instance();

    if (auto s = split.lock()) {
        auto currentParent = repo.get_scope_parent_split();
        // if (auto parent = currentParent.lock()) {
        //     logger("SPLIT")->info("NEW HELPER {} {} / PARENT: {}", s->get_id(), fmt::ptr(this), parent->get_id());
        // }
        // else {
        //     logger("SPLIT")->info("NEW HELPER {} {} / PARENT DEAD? {}", s->get_id(), fmt::ptr(this), currentParent.expired());
        // }
        s->set_parent(currentParent);
        s->begin_sprint();
        repo.set_scope_parent_split(s);
    }
}

trace::StopwatchSplitHelper::~StopwatchSplitHelper() {
    auto& repo = StopwatchRepository::get_instance();

    if (auto s = split.lock()) {
        // logger("SPLIT")->info("END HELPER {} {}", s->get_id(), fmt::ptr(this));
        s->end_sprint();
        if (auto p = s->get_parent().lock()) {
            repo.set_scope_parent_split(p);
        }
        else {
            repo.reset_scope_parent_split();
        }
    }
}

std::shared_ptr<trace::StopwatchSplit> zcl::trace::stopwatch_get(const std::string &id) {
    auto& repo = zcl::trace::StopwatchRepository::get_instance();
    return repo.get_split(id);
}

std::shared_ptr<trace::StopwatchSplitHelper> zcl::trace::stopwatch_begin(const std::string &id) {
    auto& repo = zcl::trace::StopwatchRepository::get_instance();
    auto helper = repo.begin_split(id);
    return helper;
}

void zcl::trace::stopwatch_end(const std::string &id) {
    auto& repo = zcl::trace::StopwatchRepository::get_instance();
    auto split = repo.get_split(id);
    split->end_sprint();
    if (auto p = split->get_parent().lock()) {
        repo.set_scope_parent_split(p);
    }
    else {
        repo.reset_scope_parent_split();
    }
}

std::string zcl::trace::format_as(StopwatchSplit data) {
    return std::string(data);
}

std::string zcl::trace::get_stack_trace(bool skipInternal, int skipLen, int maxLen) {
    std::ostringstream trace;

    // Print trace
    // https://stackoverflow.com/questions/691719/how-to-display-a-stack-trace-when-an-exception-is-thrown
    #ifdef _MSC_VER
        // MSVC
        // https://learn.microsoft.com/en-gb/windows/win32/api/dbghelp/nf-dbghelp-stackwalk?redirectedfrom=MSDN
        // https://www.rioki.org/2017/01/09/windows_stacktrace.html

        // Grad exec context & build current stack frame
        // https://stackoverflow.com/questions/1647930/is-it-possible-to-check-whether-you-are-building-for-64-bit-with-microsoft-c-com
        DWORD machine;
        STACKFRAME_EX frm = {};
        CONTEXT ctx = { 0 };
        ctx.ContextFlags = CONTEXT_CONTROL;

        // Fields taken notes from https://github.com/JochenKalmbach/StackWalker/blob/master/Main/StackWalker/StackWalker.cpp
        RtlCaptureContext(&ctx);
        #ifdef _M_IX86
            // Intel x86
            machine = IMAGE_FILE_MACHINE_I386;

            frm.AddrPC.Offset = ctx.Eip;
            frm.AddrPC.Mode = AddrModeFlat;
            frm.AddrFrame.Offset = ctx.Ebp;
            frm.AddrFrame.Mode = AddrModeFlat;
            frm.AddrStack.Offset = ctx.Esp;
            frm.AddrStack.Mode = AddrModeFlat;
        #elif _M_IA64
            // Itanium
            machine = IMAGE_FILE_MACHINE_IA64;

            frm.AddrPC.Offset = ctx.StIIP;
            frm.AddrPC.Mode = AddrModeFlat;
            frm.AddrFrame.Offset = ctx.IntSp;
            frm.AddrFrame.Mode = AddrModeFlat;
            frm.AddrBStore.Offset = ctx.RsBSP;
            frm.AddrBStore.Mode = AddrModeFlat;
            frm.AddrStack.Offset = ctx.IntSp;
            frm.AddrStack.Mode = AddrModeFlat;
        #elif _M_X64
            // x64 (AMD/EM)
            machine = IMAGE_FILE_MACHINE_AMD64;

            frm.AddrPC.Offset = ctx.Rip;
            frm.AddrPC.Mode = AddrModeFlat;
            frm.AddrFrame.Offset = ctx.Rsp;
            frm.AddrFrame.Mode = AddrModeFlat;
            frm.AddrStack.Offset = ctx.Rsp;
            frm.AddrStack.Mode = AddrModeFlat;
        #elif _M_ARM64
            // Arm64
            machine = IMAGE_FILE_MACHINE_ARM64;

            frm.AddrPC.Offset = ctx.Pc;
            frm.AddrPC.Mode = AddrModeFlat;
            frm.AddrFrame.Offset = ctx.Fp;
            frm.AddrFrame.Mode = AddrModeFlat;
            frm.AddrStack.Offset = ctx.Sp;
            frm.AddrStack.Mode = AddrModeFlat;
        #else
            #error "********************* UNSUPPORTED ARCHITECTURE!!! *********************"
        #endif

        auto proc = GetCurrentProcess();
        auto thread = GetCurrentThread();
        
        SymSetOptions(SYMOPT_LOAD_LINES);
        SymInitialize(proc, NULL, TRUE);

        std::string invokerFile;
        
        auto moduleBase = SymGetModuleBase(proc, frm.AddrPC.Offset);
        char moduleBuff[MAX_PATH];
        if (moduleBase && GetModuleFileName((HINSTANCE) moduleBase, moduleBuff, MAX_PATH)) {
            invokerFile = moduleBuff;
        }

        trace << "********************* [TRACE] *********************" << std::endl;
        auto traceStrs = std::vector<std::string>();
        int traceCtr = 0;
        traceStrs.reserve(maxLen);
        
        while (
            traceCtr < traceStrs.max_size() &&
            StackWalkEx(
                machine,
                proc,
                thread,
                &frm,
                &ctx,
                NULL,
                SymFunctionTableAccess,
                SymGetModuleBase,
                NULL,
                NULL
            )
        ) {
            if (skipLen > 0) {
                skipLen--;
                continue;
            }

            std::ostringstream traceLine;
            auto addr = frm.AddrPC.Offset;
            if (!addr) {
                break;
            }

            // (module / executable)
            auto moduleBase = SymGetModuleBase(proc, addr);
            char moduleBuff[MAX_PATH];
            if (moduleBase && GetModuleFileName((HINSTANCE) moduleBase, moduleBuff, MAX_PATH)) {
                if (skipInternal && invokerFile.compare(moduleBuff) != 0) {
                    continue;
                }

                traceLine << fmt::format("[file `{}`] ", moduleBuff);
            }

            // (function name & line)
            // https://learn.microsoft.com/en-gb/windows/win32/debug/retrieving-symbol-information-by-address
            char symBuff[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO sym = (PSYMBOL_INFO)symBuff;
            sym->MaxNameLen = 128;
            sym->SizeOfStruct = sizeof(SYMBOL_INFO);
            if (SymFromAddr(proc, addr,  NULL, sym)) {
                std::string name = std::string(sym->Name, sym->NameLen);

                if (
                    skipInternal
                    && (name.find("__CxxFrameHandler") != std::string::npos ||
                        name.find("_CxxThrowException") != std::string::npos ||
                        name.find("RaiseException") != std::string::npos)
                ) {
                    continue;
                }

                traceLine << fmt::format("`{}`", name);
                // GetExceptionInformation()
            }
            else {
                traceLine << "<unknown symbol>";
            }
            
            DWORD off = 0;
            IMAGEHLP_LINE line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
            if (SymGetLineFromAddr(proc, addr, &off, &line)) {
                traceLine << fmt::format("({}:{})", line.FileName, line.LineNumber);
            }
            
            // (address)
            traceLine << fmt::format(" @ 0x{:x}", addr);

            traceCtr++;
            trace << traceLine.str() << std::endl;
        }

        SymCleanup(proc);
    #else
        // (GNU/GCC)
        void *traceBuff[16];
        char **traceStrs = NULL;
        size_t traceSz = backtrace(traceBuff, 16);
        traceStrs = backtrace_symbols(traceBuff, traceSz);

        if (traceStrs == NULL) {
            trace << "********************* [TRACE FAILED! THIS IS BAD] *********************" << std::endl;
        }
        else {
            trace << "********************* [TRACE] *********************" << std::endl;
            for (int i=0; i<traceSz; i++) {
                trace << traceStrs[i] << std::endl;
            }
        }
    #endif

    return trace.str();
}

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