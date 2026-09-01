/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */

#include <iostream>
#include <fstream>
#include <string>

// LIBRARIES //
#include <zap/opengl/opengl.hpp>
#include <gfx/material.hpp>
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
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
// ----------------------------
// EXTERNAL LIBRARIES //

/** App for CPEX - T1 */
class CpexApp: public zap::OpenGlApp {
    // Scene
    std::shared_ptr<gfx::Vb<gfx::VertPosUv>> vb;
    std::shared_ptr<gfx::Shader> shd;
    std::shared_ptr<gfx::Material> mat;

    double time = 0.0;

    // ImGui
    ImGuiContext* imGuiContext = nullptr;

    void free_imgui();

    void on_setup() override;
    void on_shutdown() override;
    void on_free_resource() override;

    void on_loop_update(double dtMillis) override;
    void on_loop_render_begin(double dtMillis) override;
    void on_loop_render(double dtMillis) override;
    void on_loop_render_end(double dtMillis) override;
    // void on_loop_update_end(double dtMillis) override;

public:
    CpexApp(std::string windowTitle):
        zap::OpenGlApp(windowTitle),
        time(0) {}
    CpexApp():
        time(0) {}
    ~CpexApp() {
        free_resources();
    }

    CpexApp(const CpexApp &other) = delete; // (RAII) Disable copy
    CpexApp& operator=(const CpexApp &other) = delete; // (RAII) Disable copy

    CpexApp(CpexApp &&other): // (RAII) Move
        time(std::exchange(other.time, 0.0)),
        vb(std::move(other.vb)),
        shd(std::move(other.shd)),
        mat(std::move(other.mat)),
        imGuiContext(other.imGuiContext)
        {
        std::swap(windowTitle, other.windowTitle);
        std::swap(window, other.window);

        other.imGuiContext = nullptr;
    }
    CpexApp& operator=(CpexApp &&other) { // (RAII) Move
        if (this == &other) {
            // Self assignment, no need to move
            return *this;
        }

        free_resources();
        windowTitle = "MOVED";

        std::swap(windowTitle, other.windowTitle);
        std::swap(window, other.window);
        std::swap(vb, other.vb);
        std::swap(shd, other.shd);
        std::swap(mat, other.mat);
        std::swap(imGuiContext, other.imGuiContext);
        return *this;
    }

    void on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) override;
};