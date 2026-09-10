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

        class Stopwatch;
        class StopwatchMeta;

        class StopwatchRepository {
            std::vector<std::shared_ptr<StopwatchMeta>> watches;
            std::weak_ptr<StopwatchMeta> watchCurrent;

        public:
            std::weak_ptr<StopwatchMeta> get_scope_parent_watch() {
                return watchCurrent;
            }

            void set_scope_parent_watch(const std::string id) {
                if (id.empty()) {
                    reset_scope_parent_watch();
                }
                else {
                    auto res = get_watch(id);
                    watchCurrent = res;
                }
            }

            void reset_scope_parent_watch() {
                watchCurrent.reset();
            }

            std::shared_ptr<StopwatchMeta> get_watch(const std::string id) {
                auto res = std::find_if(
                    watches.begin(),
                    watches.end(),
                    [&] (StopwatchMeta& meta) {
                        return id == meta.id;
                    }
                );

                if (res == watches.end()) {
                    // Create new instance and return
                    auto newMeta = StopwatchMeta();
                    auto newWatch = std::make_shared<Stopwatch>(Stopwatch());
                    auto newMetaShared = std::make_shared<StopwatchMeta>(newMeta);
                    newMeta.id = id;
                    newMeta.data = newWatch;

                    watches.push_back(newMetaShared);
                    return newMetaShared;
                }
                else {
                    // Return query result
                    return *res;
                }
            }

            std::weak_ptr<StopwatchMeta> begin_watch(const std::string id) {
                auto watch = get_watch(id);
                watchCurrent = watch;

                return watch;
            }
        };

        static const StopwatchRepository watchRepo;

        class StopwatchMeta {
        public:
            std::string id = "";
            std::weak_ptr<StopwatchMeta> parent;
            std::weak_ptr<Stopwatch> data;
            std::chrono::duration<double, std::milli> timeDelta = std::chrono::duration<double, std::milli> {};

            StopwatchMeta(): StopwatchMeta(id, std::weak_ptr<Stopwatch>()) {};
            StopwatchMeta(std::string id, std::weak_ptr<Stopwatch> data):
                id(id) {};

            void begin_sprint() {
                if (auto watch = data.lock()) {
                    watch->begin_sprint();
                }
            }

            void end_sprint() {
                if (auto watch = data.lock()) {
                    watch->end_sprint();
                }
            }

            // Cast to `std::string`.
            // https://en.cppreference.com/cpp/language/cast_operator
            operator std::string() const {
                if (auto watch = data.lock()) {
                    auto duration = watch->calc_duration();
                    return fmt::format("[{}]: {:.4}ms", id, duration.count());
                }
                else {
                    return fmt::format("[{}]: <N/A>", id);
                }
            }
        };

        /**
         * Data for stopwatch, unit of (nested) performance logging/profiling.
         * Used for measuring how long a rendering function takes to execute for example.
         **/
        class Stopwatch {
            std::weak_ptr<StopwatchMeta> meta;

            std::chrono::steady_clock::time_point timeBegin = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::time_point {};
            
        public:
            Stopwatch(): Stopwatch(std::weak_ptr<StopwatchMeta>()) {};
            Stopwatch(std::weak_ptr<StopwatchMeta> meta):
                meta(meta)
                {
                auto repo = watchRepo;

                if (auto m = meta.lock()) {
                    m->parent = repo.get_scope_parent_watch();
                    repo.set_scope_parent_watch(m->id);
                }

                begin_sprint();
            };
            ~Stopwatch() {
                auto repo = watchRepo;

                if (auto m = meta.lock()) {
                    if (auto watch = m->parent.lock()) {
                        repo.set_scope_parent_watch(watch->id);
                    }
                    else {
                        repo.reset_scope_parent_watch();
                    }

                    m->timeDelta = timeEnd - timeBegin;
                }

                end_sprint();
            }
            
            void begin_sprint() {
                timeBegin = std::chrono::steady_clock::now();
            }
            void end_sprint() {
                timeEnd = std::chrono::steady_clock::now();
            }
            // void set_parent_sprint(std::weak_ptr<Stopwatch> watch) {
            //     parent = watch;
            // }
            
            std::chrono::duration<double, std::milli> calc_duration() const {
                auto dst = (timeEnd < timeBegin) ? std::chrono::steady_clock::now() : timeEnd;
                // logger("tr")->info("STOPWATCH {} DURATION: {}", name, timeBegin.time_since_epoch().count());
                return dst - timeBegin;
            }
        };

        // static StopwatchData* currentWatch = nullptr;
        // static std::vector<StopwatchData*> currentWatch;

        std::string format_as(Stopwatch data);

        std::weak_ptr<StopwatchMeta> stopwatch_get(const std::string id);
        std::weak_ptr<StopwatchMeta> stopwatch_begin(const std::string id);
        std::weak_ptr<StopwatchMeta> stopwatch_end(const std::string id);
    }
}
#endif