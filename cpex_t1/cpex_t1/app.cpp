/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>

#include <app.hpp>

// LIBRARIES //
// #include <zap/opengl/opengl.hpp>
// #include <gfx/material.hpp>
// #include <gfx/shader.hpp>
// #include <gfx/vb.hpp>
// #include <gfx/vert.hpp>
// #include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// ----------------------------
// EXTERNAL LIBRARIES //

void CpexApp::free_imgui() {
    if (imGuiContext) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imGuiContext);
    }
    imGuiContext = nullptr;
}

void CpexApp::on_setup() {
    // Setup scene
    vb = std::make_shared<gfx::Vb<gfx::VertPosUv>>();
    shd = std::make_shared<gfx::Shader>("triangle");
    mat = std::make_shared<gfx::Material>();

    // (model)
    vb->set_format(gfx::VertFormat {
        gfx::VertAttribute(0, 3, GL_DOUBLE, sizeof(double), 5, 0), // POS
        gfx::VertAttribute(1, 2, GL_DOUBLE, sizeof(double), 5, 3), // UV
    });
    vb->push_back_verts(std::vector<gfx::VertPosUv> {
        gfx::VertPosUv(-0.5, -0.5, 0.0, 0.0, 0.0),
        gfx::VertPosUv(0.5, -0.5, 0.0, 1.0, 0.0),
        gfx::VertPosUv(-0.5, 0.5, 0.0, 0.0, 1.0),
        gfx::VertPosUv(0.5, 0.5, 0.0, 1.0, 1.0),
    });
    vb->push_back_indices({
        0, 1, 2,
        1, 2, 3,
    });
    vb->build();

    // (shader)
    try {
        shd->load_shader_from("./data/triangle.vert", GL_VERTEX_SHADER);
        shd->load_shader_from("./data/triangle.frag", GL_FRAGMENT_SHADER);
        shd->link_program();
    }
    catch (std::runtime_error err) {
        throw std::runtime_error("FAILED TO PREPARE SHADER!\n" + std::string(err.what()));
    }

    // (material)
    mat->set_shader(shd);
    mat->add_uniform(
        gfx::UniformVec4("uTint", {1.0, 1.0, 1.0, 1.0})
    );

    // Setup ImGui
    imGuiContext = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Misc.
    glClearColor(1.0f, 0.8f, 0.25f, 1.0f);
}

void CpexApp::on_free_resource() {
    _logger->debug("Freeing resources...");
    free_imgui();
}

void CpexApp::on_shutdown() {
    _logger->debug("App shutdown...");
}

void CpexApp::on_loop_update(double dtMillis) {
    set_window_title(std::string("CPEX - T1 (DT: ") + std::to_string(dtMillis) + "ms)");

    time += dtMillis * 0.001;
    time = fmod(time, 1.0);
}

void CpexApp::on_loop_render_begin(double dtMillis) {
    if (!imGuiContext) {
        return;
    }

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // ImGui
    //ImGui::ShowDemoWindow(); // Show demo window! :)
    ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Render", nullptr, 0)) {
        ImGui::BulletText("time: %lf", time);
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(windowWid - 256, 0), ImGuiCond_Always);
    if (ImGui::Begin("Models", nullptr, 0)) {

    }
    ImGui::End();
}

void CpexApp::on_loop_render(double dtMillis) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw VAO
    //shd->apply_shader();
    mat->apply_material();
    if (auto uniform = mat->get_uniform<gfx::UniformVec4>("uTint")) {
        uniform->set_value({ (float) time, (float) time, (float) time, 1.0 });
    }

    vb->submit();
}

void CpexApp::on_loop_render_end(double dtMillis) {
    if (!imGuiContext) {
        return;
    }
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CpexApp::on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }
}