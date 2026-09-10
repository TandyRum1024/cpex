/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#ifndef __ZCL_GUARD
#define __ZCL_GUARD

#include <map>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>

// EXTERNAL LIBRARIES //
// ----------------------------
#include <spdlog/spdlog.h>
#include <fmt/format.h>
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

    namespace trace {
        /**
         * Stopwatch containing the data for unit of performance logging/profiling.
         * Used for measuring how long a rendering function takes to execute for example.
         **/
        struct StopwatchData {
            std::string name = "";
            std::chrono::steady_clock::time_point timeBegin = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::time_point {};

            std::vector<StopwatchData*> child;
            
        // public:
            StopwatchData(std::string name):
                name(name) {};

            void begin_sprint() {
                timeBegin = std::chrono::steady_clock::now();
                child.clear();
            }
            void end_sprint() {
                timeEnd = std::chrono::steady_clock::now();
            }
            void append_child_sprint(StopwatchData* watch) {
                child.push_back(watch);
            }

            template <typename R, typename P>
            std::chrono::duration<R, P> calc_duration() const;

            // Cast to `std::string`.
            // https://en.cppreference.com/cpp/language/cast_operator
            operator std::string() const {
                std::chrono::duration<double, std::milli> duration = calc_duration<double, std::milli>();
                return fmt::format("Timer `{}` {:.4}", name, duration.count());
            }
        };

        // static StopwatchData* currentWatch = nullptr;
        static std::vector<StopwatchData*> currentWatch;

        std::string format_as(StopwatchData data);

        StopwatchData& stopwatch(std::string id);
        StopwatchData& stopwatch_begin(std::string id);
        StopwatchData& stopwatch_end(std::string id);

        // DEFINITIONS (INCLUSION MODEL FOR TEMPLATES!) //

        template <typename R, typename P>
        std::chrono::duration<R, P> StopwatchData::calc_duration() const {
            auto dst = (timeEnd < timeBegin) ? std::chrono::steady_clock::now() : timeEnd;
            // logger("tr")->info("STOPWATCH {} DURATION: {}", name, timeBegin.time_since_epoch().count());
            return dst - timeBegin;
        }
    }
}
#endif