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

CpexApp::CpexApp(std::string windowTitle, bool isGlDebug):
    zap::OpenGlApp(windowTitle, isGlDebug) {}
CpexApp::CpexApp():
    CpexApp("WINDOW TITLE", false) {}
CpexApp::~CpexApp() {
    free_resources();
}
CpexApp::CpexApp(CpexApp &&other):
    OpenGlApp(std::move(other)),
    time(std::exchange(other.time, 0.0)),
    tfPos(other.tfPos),
    tfRot(other.tfRot),
    tfScale(other.tfScale),
    vb(std::move(other.vb)),
    shd(std::move(other.shd)),
    mat(std::move(other.mat)),
    imGuiContext(other.imGuiContext)
    {
    std::swap(windowTitle, other.windowTitle);
    std::swap(window, other.window);

    other.imGuiContext = nullptr;
}
CpexApp& CpexApp::operator=(CpexApp &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }   

    std::swap(time, other.time);
    std::swap(tfPos, other.tfPos);
    std::swap(tfRot, other.tfRot);
    std::swap(tfScale, other.tfScale);

    std::swap(vb, other.vb);
    std::swap(shd, other.shd);
    std::swap(mat, other.mat);
    std::swap(imGuiContext, other.imGuiContext);

    OpenGlApp::operator=(std::move(other));

    return *this;
}

void CpexApp::free_imgui() {
    if (imGuiContext) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imGuiContext);
    }
    imGuiContext = nullptr;
}

void CpexApp::on_setup() {
    set_vsync(false);

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
            tex2 = std::make_shared<gfx::Texture>("tex2");
    
    gfx::texhelper::texture_load_from_file_2d(*tex1, assetPath / "textest.png");
    gfx::texhelper::texture_load_from_file_2d(*tex2, assetPath / "sprtest.png");

    mat->set_shader(shd);
    mat->add_uniforms(
        gfx::UniformVec4("uTint", {1.0, 1.0, 1.0, 1.0}),
        gfx::UniformMat4("uMatTf", glm::mat4(1.0f)),
        gfx::UniformSampler2D("uBaseTexture", tex1),
        gfx::UniformSampler2D("uOverTexture", tex2, GL_LINEAR, GL_CLAMP_TO_BORDER)
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

void imgui_draw_stopwatch_node(const std::shared_ptr<zcl::trace::StopwatchSplitNode> node) {
    if (!node) {
        return;
    }

    auto flags = ImGuiTreeNodeFlags_None
                        | (node->parent.expired() ? ImGuiTreeNodeFlags_DefaultOpen : 0)
                        | (node->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    auto totalTime = 0.0;
    
    if (ImGui::TreeNodeEx(fmt::format("watch: {0} ({1:.2f}ms)###{0}", node->id, node->duration.count()).c_str(), flags)) {
        for (auto&& child: node->children) {
            totalTime += child->duration.count();
            imgui_draw_stopwatch_node(child);
        }
        ImGui::TreePop();
    }
    if (!node->children.empty()) {
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5), fmt::format("({}: {:.2f}ms total w/ overhead)", node->id, totalTime).c_str());
    }
}

void CpexApp::on_loop_debug_ui(double dtMillis) {
    if (!imGuiContext) {
        return;
    }

    // ImGui
    //ImGui::ShowDemoWindow(); // Show demo window! :)
    ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    if (ImGui::Begin("Scene", nullptr, 0)) {
        ImGui::BulletText("time: %lf (dt: %lf)", time, dtMillis);

        auto& repo = zcl::trace::StopwatchRepository::get_instance();
        auto roots = repo.get_all_splits_and_childs();
        
        ImGui::BulletText("[STOPWATCH (%d)]", roots.size());
        for (auto&& root: roots) {
            imgui_draw_stopwatch_node(root);
        }

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
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CpexApp::on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }
}