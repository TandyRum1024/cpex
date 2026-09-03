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