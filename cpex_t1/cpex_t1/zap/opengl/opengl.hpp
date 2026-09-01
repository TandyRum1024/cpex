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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace zap {
    void _common_window_resize(GLFWwindow* win, int wid, int hei);
    void _common_window_key(GLFWwindow* win, int key, int scancode, int action, int mods);
    void APIENTRY _common_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    /** OpenGL app lifecycle abstraction. */
    class OpenGlApp : public App {
    protected:
        std::shared_ptr<spdlog::logger> _logger;

        std::string windowTitle;
        GLFWwindow* window = nullptr;
        std::chrono::steady_clock::time_point dtPrev;

        OpenGlApp(std::string windowTitle):
            windowTitle(windowTitle) {
            prepare_logger();
        }
        OpenGlApp() {
            prepare_logger();
        }
        virtual ~OpenGlApp() {
            free_resources();
        }

        OpenGlApp(const OpenGlApp &other) = delete; // (RAII) Disable copy
        OpenGlApp& operator=(const OpenGlApp &other) = delete; // (RAII) Disable copy

        OpenGlApp(OpenGlApp &&other): // (RAII) Move
            windowTitle(std::move(other.windowTitle)),
            window(other.window),
            _logger(std::move(other._logger)) {
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
            std::swap(_logger, other._logger);
            return *this;
        }

        void free_resources() {
            on_free_resource();

            if (window) {
                glfwDestroyWindow(window);
            }
            window = nullptr;
        }

        void prepare_logger() {
            _logger = spdlog::get("APP");
            if (!_logger) {
                _logger = spdlog::stdout_color_mt("APP");
                // _logger->set_level(spdlog::level::debug);
            }
        }

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