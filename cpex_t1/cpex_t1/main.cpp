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
// ----------------------------
// EXTERNAL LIBRARIES //

int main() {
    CpexApp app;

    try {
        app.boot();
    }
    catch (std::exception e) {
        std::cerr << "[!!!] UNHANDLED EXCEPTION!" << std::endl << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "[!!!] FATAL EXCEPTION HAS OCCURED! AND NOW THE APP WILL TERMINATE. BYE" << std::endl;
    }

    std::cout << "APP TERMINATING!" << std::endl;
    return 0;
}