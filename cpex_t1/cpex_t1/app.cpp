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

void CpexApp::on_setup() {
    // Setup scene
    // (model)
    vb.set_format(gfx::VertFormat {
        gfx::VertAttribute(0, 3, GL_DOUBLE, sizeof(double), 5, 0), // POS
        gfx::VertAttribute(1, 2, GL_DOUBLE, sizeof(double), 5, 3), // UV
    });
    vb.push_back_verts(std::vector<gfx::VertPosUv> {
        gfx::VertPosUv(-0.5, -0.5, 0.0, 0.0, 0.0),
        gfx::VertPosUv(0.5, -0.5, 0.0, 1.0, 0.0),
        gfx::VertPosUv(-0.5, 0.5, 0.0, 0.0, 1.0),
        gfx::VertPosUv(0.5, 0.5, 0.0, 1.0, 1.0),
    });
    vb.push_back_indices({
        0, 1, 2,
        1, 2, 3,
    });
    vb.build();

    // (shader)
    shd = gfx::Shader("triangle");
    try {
        shd.load_shader_from("./data/triangle.vert", GL_VERTEX_SHADER);
        shd.load_shader_from("./data/triangle.frag", GL_FRAGMENT_SHADER);
        shd.link_program();
    }
    catch (std::runtime_error err) {
        throw std::runtime_error("FAILED TO PREPARE SHADER!\n" + std::string(err.what()));
    }

    // Misc.
    glClearColor(1.0f, 0.8f, 0.25f, 1.0f);
}

void CpexApp::on_loop_update(double dtMillis) {
    set_window_title(std::string("DT: ") + std::to_string(dtMillis) + "ms");
}

void CpexApp::on_loop_render(double dtMillis) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw VAO
    shd.use_shader();
    vb.submit();
}