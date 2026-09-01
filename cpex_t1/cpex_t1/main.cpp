/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>

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
    CpexApp app("CPEX - T1");

    spdlog::stdout_color_mt("PRG");
    auto logger = spdlog::get("PRG");

    try {
        logger->info("Booting app...");
        app.boot();
    }
    catch (std::exception e) {
        logger->error("UNHANDLED EXCEPTION!:\n{}", e.what());
    }
    catch (...) {
        logger->error("FATAL EXCEPTION HAS OCCURED! AND NOW THE APP WILL TERMINATE. BYE");
    }

    logger->info("Terminating app!");
    return 0;
}