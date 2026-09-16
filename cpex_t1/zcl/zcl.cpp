/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#include <iostream>
#include <stdexcept>
#include <filesystem>

// LIBRARY
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// OS
// https://github.com/cpredef/predef/blob/master/OperatingSystems.md
#if defined(_WIN32)
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
// ----------------------------
// EXTERNAL LIBRARIES //

using namespace zcl;

std::string file::read_file_to_string(std::filesystem::path filePath) {
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

std::filesystem::path file::get_exec_path() {
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

std::weak_ptr<trace::StopwatchSplit> trace::StopwatchRepository::get_scope_parent_split() {
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

std::shared_ptr<trace::StopwatchSplit> trace::StopwatchRepository::get_split(const std::string id) {
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

std::shared_ptr<trace::StopwatchSplitHelper> trace::StopwatchRepository::begin_split(const std::string id) {
    auto splitPtr = get_split(id);
    auto helper = std::make_shared<StopwatchSplitHelper>(StopwatchSplitHelper(splitPtr));

    return helper;
}

std::vector<std::shared_ptr<trace::StopwatchSplitNode>> trace::StopwatchRepository::get_all_splits_and_childs() {
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
trace::StopwatchSplitNode::StopwatchSplitNode(std::string id): id(id) {}

trace::StopwatchSplitNode::operator std::string() const {
    return fmt::format("[{}]: {:.4}ms", id, duration.count());
}

trace::StopwatchSplit::StopwatchSplit(): StopwatchSplit("") {}
trace::StopwatchSplit::StopwatchSplit(std::string id): id(id) {
    // logger("SPLIT")->info("NEW SPLIT {} {}", id, fmt::ptr(this));
}

std::weak_ptr<trace::StopwatchSplit> trace::StopwatchSplit::get_parent() {
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

std::shared_ptr<trace::StopwatchSplit> zcl::trace::stopwatch_get(const std::string id) {
    auto& repo = zcl::trace::StopwatchRepository::get_instance();
    return repo.get_split(id);
}

std::shared_ptr<trace::StopwatchSplitHelper> zcl::trace::stopwatch_begin(const std::string id) {
    auto& repo = zcl::trace::StopwatchRepository::get_instance();
    auto helper = repo.begin_split(id);
    return helper;
}

void zcl::trace::stopwatch_end(const std::string id) {
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
