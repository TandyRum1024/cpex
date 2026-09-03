/**
 * zap::opengl - OpenGL app frame(?)work.
 * ZIK@MMXXVI
 */

#include <zap/opengl/opengl.hpp>

using namespace zap;

void zap::_common_window_resize(GLFWwindow* win, int wid, int hei) {
    // OpenGlApp* app = (OpenGlApp*) glfwGetWindowUserPointer(win);
    OpenGlApp* app = static_cast<OpenGlApp*>(glfwGetWindowUserPointer(win));
    app->on_window_resize(win, wid, hei);
}

void zap::_common_window_key(GLFWwindow* win, int key, int scancode, int action, int mods) {
    OpenGlApp* app = static_cast<OpenGlApp*>(glfwGetWindowUserPointer(win));
    app->on_window_key(win, key, scancode, action, mods);
}

void APIENTRY zap::_common_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    }
    std::cout << std::endl;
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
};

void OpenGlApp::boot() {
    if (!glfwInit()) {
        throw std::runtime_error("[GLFW] GLFW INIT FAILED!");
    }

    _logger->info("GLFW: Activating debug context...");
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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
    int contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

    if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        _logger->warn("GLFW: DEBUG CONTEXT ACTIVATED!");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(_common_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
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

    // Setup
    on_setup();

    // Begin loop
    // https://gameprogrammingpatterns.com/game-loop.html
    dtPrev = std::chrono::high_resolution_clock::now();
    std::chrono::steady_clock::time_point dtNow = dtPrev;
    
    while (!glfwWindowShouldClose(window)) {
        dtPrev = dtNow;
        dtNow = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> dtMillis = dtNow - dtPrev;

        glfwPollEvents();
        if (!isRenderReady) {
            isRenderReady = true;
        }
        
        // Logic
        on_loop_update(dtMillis.count());

        // Render
        on_loop_render_begin(dtMillis.count());
        on_loop_render(dtMillis.count());
        on_loop_render_end(dtMillis.count());
        glfwSwapBuffers(window);

        // Finishing logic
        on_loop_update_end(dtMillis.count());
    }
    
    on_shutdown();

    _logger->info("GLFW: Terminating GLFW!");
    glfwTerminate();
}

void OpenGlApp::shutdown() {
    glfwSetWindowShouldClose(window, true);
}

void OpenGlApp::set_window_title(std::string windowTitle) {
    this->windowTitle = windowTitle;
    glfwSetWindowTitle(window, windowTitle.c_str());
}

void OpenGlApp::on_window_resize(GLFWwindow* win, int wid, int hei) {
    _logger->debug("GLFW: New window size: ({}, {})", wid, hei);
    
    windowWid = wid;
    windowHei = hei;
    glViewport(0, 0, wid, hei);

    if (isRenderReady) {
        std::chrono::steady_clock::time_point dtNow = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> dtMillis = dtNow - dtPrev;

        on_loop_render_begin(dtMillis.count());
        on_loop_render(dtMillis.count());
        on_loop_render_end(dtMillis.count());
        glfwSwapBuffers(win);
    }
}
