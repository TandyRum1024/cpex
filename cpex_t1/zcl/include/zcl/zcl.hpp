/**
 * zcl - Common helper library
 * ZIK@MMXXVI
 */

#ifndef __ZCL_GUARD
#define __ZCL_GUARD

#include <map>
#include <exception>
#include <utility>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <algorithm>

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
         * Data for stopwatch "split", unit of (nested) performance logging/profiling.
         * Used for measuring how long a rendering function takes to execute for example.
         */
        class StopwatchSplit {
            std::string id = "";
            std::weak_ptr<StopwatchSplit> parentSplit;
            
            std::chrono::steady_clock::time_point timeBegin = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::time_point {};
            std::chrono::duration<double, std::milli> timeDelta = std::chrono::duration<double, std::milli> {};
            
        public:
            StopwatchSplit();
            StopwatchSplit(std::string id);

            std::weak_ptr<StopwatchSplit> get_parent();
            void set_parent(std::weak_ptr<StopwatchSplit> split);
            std::string get_id();
            void begin_sprint();
            void end_sprint();

            std::chrono::duration<double, std::milli> calc_duration() const;

            // Cast to `std::string`.
            // https://en.cppreference.com/cpp/language/cast_operator
            operator std::string() const;
        };

        /** Helper class that uses RAII-like pattern to automatically begin and end the split. */
        class StopwatchSplitHelper {
            std::weak_ptr<StopwatchSplit> split;

        public:
            StopwatchSplitHelper(std::weak_ptr<StopwatchSplit> split);
            StopwatchSplitHelper(StopwatchSplitHelper&& other);
            ~StopwatchSplitHelper();
        };

        /** Convenience struct for containing "tree" of split. Mainly used for plugging into ImGui and likes. */
        struct StopwatchSplitNode {
            // (deliberately kept as struct since this works like a "data" class in Kotlin for example so its no use to make it a class and have it all public and/or make getter/setters for all fields)
            std::string id = "";
            std::weak_ptr<StopwatchSplitNode> parent;
            std::vector<std::shared_ptr<StopwatchSplitNode>> children;
            std::chrono::duration<double, std::milli> duration;
        
            StopwatchSplitNode();
            StopwatchSplitNode(std::string id);
            operator std::string() const;
        };

        /** Repository that manages splits. */
        class StopwatchRepository {
            std::vector<std::shared_ptr<StopwatchSplit>> splits;
            std::weak_ptr<StopwatchSplit> splitCurrent;

        public:
            // StopwatchRepository() {
            //     zcl::logger("SPLIT")->info("NEW REPO {}", fmt::ptr(this));
            // }
            // ~StopwatchRepository() {
            //     zcl::logger("SPLIT")->info("DEL REPO {}", fmt::ptr(this));
            // }

            static StopwatchRepository& get_instance();
            std::weak_ptr<StopwatchSplit> get_scope_parent_split();

            void set_scope_parent_split(std::weak_ptr<StopwatchSplit> split);
            void reset_scope_parent_split();

            std::shared_ptr<StopwatchSplit> get_split(const std::string id);
            std::shared_ptr<StopwatchSplitHelper> begin_split(const std::string id);

            std::vector<std::shared_ptr<StopwatchSplitNode>> get_all_splits_and_childs();
        };

        // static StopwatchData* currentWatch = nullptr;
        // static std::vector<StopwatchData*> currentWatch;

        std::string format_as(StopwatchSplit data);

        std::shared_ptr<StopwatchSplit> stopwatch_get(const std::string id);
        std::shared_ptr<StopwatchSplitHelper> stopwatch_begin(const std::string id);
        void stopwatch_end(const std::string id);

        std::string get_stack_trace(bool skipInternal = true, int skipLen = 0, int maxLen = 16);
    }
}
#endif