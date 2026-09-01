/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>

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

/** App for CPEX - T1 */
class CpexApp: public zap::OpenGlApp {
    gfx::Vb<gfx::VertPosUv> vb;
    gfx::Shader shd;

    void on_setup() override;
    void on_loop_update(double dtMillis) override;
    void on_loop_render(double dtMillis) override;
};