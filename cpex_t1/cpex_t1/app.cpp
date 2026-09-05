/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>

#include <app.hpp>

// LIBRARIES //
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// GLM
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
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
    // Relative path
    auto assetPath = zcl::file::get_exec_path().parent_path() / "data";
    _logger->info("Detected asset path: {}", assetPath.string());

    // Setup scene
    time = 0;
    tfPos = glm::vec3(0.0);
    tfRot = glm::vec3(0.0);
    tfScale = glm::vec3(1.0);

    // vb = std::make_shared<gfx::Vb<gfx::VertPosUv>>();
    auto vb1 = gfx::Vb<gfx::VertPosUv>();
    auto vb2 = gfx::Vb<gfx::VertPosUv>();

    // (model)
    vb1.set_format(gfx::VertPosUv::format);
    vb1.push_back_verts(std::vector<gfx::VertPosUv> {
        gfx::VertPosUv({-0.5, -0.5, 0.0}, {0.0, 0.0}),
        gfx::VertPosUv({0.5, -0.5, 0.0}, {1.0, 0.0}),
        gfx::VertPosUv({-0.5, 0.5, 0.0}, {0.0, 1.0}),
        gfx::VertPosUv({0.5, 0.5, 0.0}, {1.0, 1.0}),
    });
    vb1.push_back_indices({
        0, 1, 2,
        1, 2, 3,
    });
    vb1.build();

    vb = std::make_shared<gfx::Vb<gfx::VertPosUv>>(std::move(vb1));
    shd = std::make_shared<gfx::Shader>("triangle");
    mat = std::make_shared<gfx::Material>();

    // (shader)
    try {
        shd->load_shader_from(assetPath / "triangle.vert", GL_VERTEX_SHADER);
        shd->load_shader_from(assetPath / "triangle.frag", GL_FRAGMENT_SHADER);
        shd->link_program();
    }
    catch (std::runtime_error err) {
        throw std::runtime_error("FAILED TO PREPARE SHADER!\n" + std::string(err.what()));
    }

    // (material)
    auto    tex1 = std::make_shared<gfx::Texture>("tex1"),
            tex2 = std::make_shared<gfx::Texture>(gfx::Texture("tex2")); // move
    
    gfx::texture_load_from_file_2d(*tex1, assetPath / "textest.png");
    gfx::texture_load_from_file_2d(*tex2, assetPath / "sprtest.png");

    mat->set_shader(shd);
    mat->add_uniforms(
        gfx::UniformVec4("uTint", {1.0, 1.0, 1.0, 1.0}),
        gfx::UniformMat4("uMatTf", glm::mat4(1.0f)),
        gfx::UniformSampler2D("uBaseTexture", tex1),
        gfx::UniformSampler2D("uOverTexture", tex2, GL_LINEAR, GL_REPEAT)
    );

    // Setup ImGui
    imGuiContext = ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Misc.
    glClearColor(1.0f, 0.8f, 0.25f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _logger->info("Setup done");
}

void CpexApp::on_free_resource() {
    _logger->debug("Freeing resources...");
    free_imgui();
}

void CpexApp::on_shutdown() {
    _logger->debug("App shutdown...");
}

void CpexApp::on_loop_update(double dtMillis) {
    set_window_title(std::string("CT1 (DT: ") + std::to_string(dtMillis) + "ms)");

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
    if (ImGui::Begin("Scene", nullptr, 0)) {
        ImGui::BulletText("render time: %lf", time);
        ImGui::DragFloat3("pos", glm::value_ptr(tfPos));
        ImGui::DragFloat3("rot", glm::value_ptr(tfRot));
        ImGui::DragFloat3("scale", glm::value_ptr(tfScale));
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
    if (auto uniform = mat->get_uniform<gfx::UniformMat4>("uMatTf")) {
        auto tf = glm::translate(
            glm::rotate(
                glm::rotate(
                    glm::rotate(
                        glm::scale(glm::mat4(1.0), tfScale),
                        glm::radians(tfRot.x),
                        glm::vec3(1.0, 0.0, 0.0)
                    ),
                    glm::radians(tfRot.y),
                    glm::vec3(0.0, 1.0, 0.0)
                ),
                glm::radians(tfRot.z),
                glm::vec3(0.0, 0.0, 1.0)
            ),
            tfPos
        );
        
        uniform->set_value(tf);
    }

    if (vb) {
        vb->submit(GL_TRIANGLES, 0);
    }
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