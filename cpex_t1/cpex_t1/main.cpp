/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <exception>

#include <app.hpp>

// LIBRARIES //
#include <zap/opengl/opengl.hpp>
#include <gfx/shader.hpp>
#include <gfx/vb.hpp>
#include <gfx/vert.hpp>
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// ----------------------------
// EXTERNAL LIBRARIES //

int main() {
    CpexApp app("CT1", true);

    spdlog::stdout_color_mt("PRG");
    auto logger = spdlog::get("PRG");

    std::set_terminate([] () {
        auto trace = zcl::trace::get_stack_trace();
        auto logger = spdlog::get("PRG");
        auto eptr = std::exception_ptr(std::current_exception());

        logger->error("********************* UNHANDLED EXCEPTION! *********************");
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
            else {
                logger->error("(NO EXCEPTION DATA WAS FOUND!)");
            }
        }
        catch (std::exception e) {
            logger->error(fmt::format("exception: {}", e.what()));
        }
        catch (...) {
            logger->error("FATAL EXCEPTION HAS OCCURED! AND NOW THE APP WILL TERMINATE. BYE");
        }
        
        logger->error(trace);
        std::exit(EXIT_FAILURE);
    });

    logger->info("Booting app...");
    app.boot();
    logger->info("Terminating app!");
    return 0;
}