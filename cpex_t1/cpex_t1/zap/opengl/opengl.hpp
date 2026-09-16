/**
 * zap::opengl - OpenGL app frame(?)work.
 * ZIK@MMXXVI
 */

#ifndef __ZAP_OPENGL_GUARD
#define __ZAP_OPENGL_GUARD

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

// LIBRARIES //
#include <zap/zap.hpp>
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

namespace zap {
    void _common_window_resize(GLFWwindow* win, int wid, int hei);
    void _common_window_key(GLFWwindow* win, int key, int scancode, int action, int mods);
    void APIENTRY _common_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    /** OpenGL app lifecycle abstraction. */
    class OpenGlApp : public App {
    protected:
        std::shared_ptr<spdlog::logger> _logger;

        bool isRenderReady;
        std::string windowTitle;
        int windowWid;
        int windowHei;
        GLFWwindow* window = nullptr;

        OpenGlApp(std::string windowTitle);
        OpenGlApp();
        virtual ~OpenGlApp();

        OpenGlApp(const OpenGlApp &other) = delete; // (RAII) Disable copy
        OpenGlApp& operator=(const OpenGlApp &other) = delete; // (RAII) Disable copy

        OpenGlApp(OpenGlApp &&other); // (RAII) Move
        OpenGlApp& operator=(OpenGlApp &&other); // (RAII) Move

        void free_resources();

        /** Updates current windows title. */
        void set_window_title(std::string windowTitle);
        /** Called on destructor, move, etc when the class is no longer being used. */
        virtual void on_free_resource() {};
        /** Called on program shutdown, before GLFW is terminated. */
        virtual void on_shutdown() {};
        
        /** Called on update in render loop. */
        virtual void on_loop_update(double dtMillis) = 0;
        /** Called on screen render in render loop. */
        virtual void on_loop_render(double dtMillis) = 0;
        /** Called before render in render loop. */
        virtual void on_loop_render_begin(double dtMillis) {};
        /** Called after render in render loop. */
        virtual void on_loop_render_end(double dtMillis) {};
        /** Called after update & render in render loop. */
        virtual void on_loop_update_end(double dtMillis) {};

        /** Called after everything right before swap. */
        virtual void on_loop_debug_ui(double dtMillis) {};

    public:
        /** Called on window resize. */
        virtual void on_window_resize(GLFWwindow* win, int wid, int hei);
        /** Called on window resize. */
        virtual void on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {};
        
        void boot() override;
        void shutdown() override;
    };
}
#endif