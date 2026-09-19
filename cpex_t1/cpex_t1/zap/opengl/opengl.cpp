/**
 * zap::opengl - OpenGL app frame(?)work.
 * ZIK@MMXXVI
 */

#include <zap/opengl/opengl.hpp>

using namespace zap;

void _common_window_resize(GLFWwindow* win, int wid, int hei) {
    // OpenGlApp* app = (OpenGlApp*) glfwGetWindowUserPointer(win);
    OpenGlApp* app = static_cast<OpenGlApp*>(glfwGetWindowUserPointer(win));
    app->on_window_resize(win, wid, hei);
}

void _common_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {
    OpenGlApp* app = static_cast<OpenGlApp*>(glfwGetWindowUserPointer(win));
    app->on_window_key(win, key, scancode, action, mods);
}

void APIENTRY _common_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
    // Ignore minor id/codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    auto prefix = "\t\t\t\t\t\t";
    std::ostringstream msg;

    msg << fmt::format("\tOPENGL DEBUG MSG ({}):", id) << std::endl;
    msg << prefix << "======================================" << std::endl;

    msg << prefix << fmt::format("\"{}\"", message) << std::endl;
    msg << prefix << "\t* SRC = ";
    switch (source) {
        case GL_DEBUG_SOURCE_API:             msg << "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   msg << "WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: msg << "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     msg << "THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     msg << "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER:           msg << "OTHER"; break;
    }
    msg << std::endl;

    msg << prefix << "\t* TYPE = ";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               msg << "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msg << "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  msg << "UNDEFINED_BEHAVIOR"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         msg << "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         msg << "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER:              msg << "MARKER"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          msg << "PUSH_GROUP"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           msg << "POP_GROUP"; break;
        case GL_DEBUG_TYPE_OTHER:               msg << "OTHER"; break;
    }
    msg << std::endl << std::endl;
    msg << prefix << "======================================" << std::endl;

    auto msgStr = msg.str();
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            zcl::logger("GL")->critical(msgStr);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            zcl::logger("GL")->error(msgStr);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            zcl::logger("GL")->warn(msgStr);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            zcl::logger("GL")->info(msgStr);
            break;
    }
};

OpenGlApp::OpenGlApp(const std::string windowTitle, bool isGlDebug):
    isGlDebug(isGlDebug),
    isRenderReady(false),
    isVsync(true),
    windowTitle(windowTitle),
    windowWid(1280),
    windowHei(720),
    _logger(zcl::logger("APP"))
    {};

OpenGlApp::OpenGlApp():
    OpenGlApp("WINDOW TITLE", false) {};

OpenGlApp::~OpenGlApp() {
    free_resources();
}

OpenGlApp::OpenGlApp(OpenGlApp &&other):
    _logger(std::move(other._logger)),
    isGlDebug(std::exchange(other.isGlDebug, false)),
    isRenderReady(std::exchange(other.isRenderReady, false)),
    isVsync(std::exchange(other.isVsync, false)),
    windowTitle(std::move(other.windowTitle)),
    windowWid(std::exchange(other.windowWid, 0)),
    windowHei(std::exchange(other.windowHei, 0)),
    window(other.window) {
    other.window = nullptr;
}

OpenGlApp& OpenGlApp::operator=(OpenGlApp &&other) {
    if (this == &other) {
        // Self assignment, no need to move
        return *this;
    }
    
    windowTitle = "MOVED";
    std::swap(_logger, other._logger);
    std::swap(isGlDebug, other.isGlDebug);
    std::swap(isRenderReady, other.isRenderReady);
    std::swap(isVsync, other.isVsync);
    std::swap(windowTitle, other.windowTitle);
    std::swap(windowWid, other.windowWid);
    std::swap(windowHei, other.windowHei);
    std::swap(window, other.window);
    
    other.free_resources();

    return *this;
}

void OpenGlApp::free_resources() {
    on_free_resource();

    if (window) {
        glfwDestroyWindow(window);
    }
    window = nullptr;
}

void OpenGlApp::boot() {
    if (!glfwInit()) {
        throw std::runtime_error("[GLFW] GLFW INIT FAILED!");
    }

    _logger->info("GLFW: Activating debug context...");
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, isGlDebug);

    _logger->info("GLFW: Creating window...");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // https://sourceforge.net/p/predef/wiki/OperatingSystems/
    #if (defined(__APPLE__) && defined(__MACH__)) || defined(Macintosh) || defined(macintosh)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(windowWid, windowHei, windowTitle.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("[GLFW] CREATING WINDOW FAILED!");
    }
    glfwMakeContextCurrent(window);

    // https://www.glfw.org/faq.html#216---how-do-i-use-c-methods-as-callbacks
    // Pass this instance to user pointer & abuse(?) it to call callbacks
    glfwSetWindowUserPointer(window, this);

    auto version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        glfwTerminate();
        throw std::runtime_error("[GLAD] FAILED TO INIT OPENGL!");
    }
    else {
        _logger->info("GLAD: OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    }

    // (setup callbacks)
    glfwSetFramebufferSizeCallback(window, _common_window_resize);
    glfwSetKeyCallback(window, _common_window_key);

    // (debug callbacks)
    if (isGlDebug) {
        int contextFlags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
    
        if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            _logger->warn("GLFW: DEBUG CONTEXT ACTIVATED!");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(_common_debug_output, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    // Set render viewport size
    int winWid = -1, winHei = -1;
    glfwGetWindowSize(window, &winWid, &winHei);
    if (winWid > 0 && winHei > 0) {
        // glViewport(0, 0, winWid, winHei);
        _common_window_resize(window, winWid, winHei);
    }
    else {
        glfwTerminate();
        throw std::runtime_error("INVALID WINDOW SIZE: (" + std::to_string(winWid) + ", " + std::to_string(winHei) + ")");
    }

    // Set VSync
    // glfwSwapInterval(1);

    // Setup
    on_setup();

    // Begin loop
    // https://gameprogrammingpatterns.com/game-loop.html
    //auto timeFramePrev = std::chrono::steady_clock::now();
    auto timeFramePrev = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        auto timeFrameNow = glfwGetTime();
        auto delta = timeFrameNow - timeFramePrev; // in seconds
        auto dtMillis = delta * 1000;
        timeFramePrev = timeFrameNow;

        glfwPollEvents();
        if (!isRenderReady) {
            isRenderReady = true;
        }
        
        // Begin frame
        {
            auto st = zcl::trace::stopwatch_begin("frame");
    
            // Logic
            {
                auto st = zcl::trace::stopwatch_begin("update");
                on_loop_update(dtMillis);
            }
            
            // Render
            {
                auto st = zcl::trace::stopwatch_begin("render_all");
    
                {
                    auto st = zcl::trace::stopwatch_begin("render_begin");
                    on_loop_render_begin(dtMillis);
                }
                {
                    auto st = zcl::trace::stopwatch_begin("render");
                    on_loop_render(dtMillis);
                }
                {
                    auto st = zcl::trace::stopwatch_begin("render_end");
                    on_loop_render_end(dtMillis);
                }
            }
    
            // Logic end
            {
                auto st = zcl::trace::stopwatch_begin("update_end");
                on_loop_update_end(dtMillis);
            }
        }
        // Debug and all
        {
            auto st = zcl::trace::stopwatch_begin("debug_ui");
            on_loop_debug_ui(dtMillis);
        }

        glfwSwapBuffers(window);
    }
    
    on_shutdown();

    _logger->info("GLFW: Terminating GLFW!");
    glfwTerminate();
}

void OpenGlApp::shutdown() {
    glfwSetWindowShouldClose(window, true);
}

void OpenGlApp::set_vsync(bool isVsync) {
    this->isVsync = isVsync;
    glfwSwapInterval(isVsync ? 1 : 0);
}

void OpenGlApp::set_window_title(const std::string windowTitle) {
    this->windowTitle = windowTitle;
    glfwSetWindowTitle(window, windowTitle.c_str());
}

void OpenGlApp::on_window_resize(GLFWwindow* win, int wid, int hei) {
    _logger->debug("GLFW: New window size: ({}, {})", wid, hei);
    
    windowWid = wid;
    windowHei = hei;
    glViewport(0, 0, wid, hei);

    if (isRenderReady) {
        on_loop_render_begin(0);
        on_loop_render(0);
        on_loop_render_end(0);
        on_loop_debug_ui(0);
        glfwSwapBuffers(win);
    }
}
