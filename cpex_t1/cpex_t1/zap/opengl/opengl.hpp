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

#include <zap/zap.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace zap {
    void _common_window_resize(GLFWwindow* win, int wid, int hei);
    void _common_window_key(GLFWwindow* win, int key, int scancode, int action, int mods);
    void APIENTRY _common_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    /** OpenGL app lifecycle abstraction. */
    class OpenGlApp : public App {
    protected:
        std::string windowTitle;
        GLFWwindow* window;
        std::chrono::steady_clock::time_point dtPrev;

        OpenGlApp(std::string windowTitle):
            windowTitle(windowTitle) {}
        OpenGlApp() {}
        ~OpenGlApp() {
            free_resources();
        }

        OpenGlApp(const OpenGlApp &other) = delete; // (RAII) Disable copy
        OpenGlApp& operator=(const OpenGlApp &other) = delete; // (RAII) Disable copy

        OpenGlApp(OpenGlApp &&other): // (RAII) Move
            windowTitle(std::move(other.windowTitle)),
            window(other.window) {
            other.window = nullptr;
        }
        OpenGlApp& operator=(OpenGlApp &&other) { // (RAII) Move
            if (this == &other) {
                // Self assignment, no need to move
                return *this;
            }
            
            free_resources();
            windowTitle = "MOVED";

            std::swap(windowTitle, other.windowTitle);
            std::swap(window, other.window);
            return *this;
        }

        void free_resources() {
            if (window) {
                glfwDestroyWindow(window);
            }
            window = nullptr;
        }

        void set_window_title(std::string windowTitle) {
            this->windowTitle = windowTitle;
            glfwSetWindowTitle(window, windowTitle.c_str());
        }

        /** Called on update in render loop. */
        virtual void on_loop_update(double dtMillis) = 0;
        /** Called on screen render in render loop. */
        virtual void on_loop_render(double dtMillis) = 0;

    public:
        /** Called on window resize. */
        void on_window_resize(GLFWwindow* win, int wid, int hei);
        /** Called on window resize. */
        void on_window_key(GLFWwindow* win, int key, int scancode, int action, int mods);

        void boot();
        void shutdown();
    };
}
#endif