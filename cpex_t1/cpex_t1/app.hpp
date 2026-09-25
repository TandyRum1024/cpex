/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>

// LIBRARIES //
#include <zap/opengl/opengl.hpp>
#include <gfx/texture.hpp>
#include <gfx/material.hpp>
#include <gfx/shader.hpp>
#include <gfx/vb.hpp>
#include <gfx/vert.hpp>
#include <zmd2/model.hpp>
#include <zcl/zcl.hpp>

// EXTERNAL LIBRARIES //
// ----------------------------
// OpenGL: GLAD
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
// GLM
#include <glm/vec3.hpp>
// ----------------------------
// EXTERNAL LIBRARIES //

/** App for CPEX - T1 */
class CpexApp: public zap::OpenGlApp {
    // Scene
    // std::shared_ptr<gfx::Vb> vb;
    // std::shared_ptr<gfx::Shader> shd;
    // std::shared_ptr<gfx::Material> matBase;
    // std::shared_ptr<gfx::Material> mat;

    std::shared_ptr<zmd2::Model> model1;
    std::shared_ptr<zmd2::Model> model2;

    double time;

    glm::vec3 tfPos;
    glm::vec3 tfRot;
    glm::vec3 tfScale;

    // ImGui
    ImGuiContext* imGuiContext = nullptr;

    // Renderer
    gfx::TextureManager texManager;

    void free_imgui();

    void on_setup() override;
    void on_shutdown() override;
    void on_free_resource() override;

    void on_loop_update(double dtMillis) override;
    void on_loop_render_begin(double dtMillis) override;
    void on_loop_render(double dtMillis) override;
    // void on_loop_render_end(double dtMillis) override;
    // void on_loop_update_end(double dtMillis) override;
    void on_loop_debug_ui(double dtMillis) override;

public:
    CpexApp(const std::string windowTitle, bool isGlDebug = false);
    CpexApp();
    ~CpexApp();

    CpexApp(const CpexApp &other) = delete; // (RAII) Disable copy
    CpexApp& operator=(const CpexApp &other) = delete; // (RAII) Disable copy

    CpexApp(CpexApp &&other); // (RAII) Move
    CpexApp& operator=(CpexApp &&other); // (RAII) Move

    void on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) override;
};