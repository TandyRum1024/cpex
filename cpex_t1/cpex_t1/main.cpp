/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */
#include <iostream>
#include <fstream>
#include <string>

// LIBRARIES //
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
// ----------------------------
// EXTERNAL LIBRARIES //

void process_input(GLFWwindow* win) {
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }
}

void on_window_resize(GLFWwindow* win, int wid, int hei) {
    glViewport(0, 0, wid, hei);
    std::cout << "NEW WINDOW SIZE: (" << wid << ", " << hei << ")" << std::endl;
}

// https://learnopengl.com/In-Practice/Debugging
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
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
}

int main() {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cerr << "[GLFW] GLFW INIT FAILED!" << std::endl;
        return -1;
    }

    std::cout << "[GLFW] TRY ACTIVATING DEBUG CONTEXT..." << std::endl;
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    std::cout << "[GLFW] CREATING WINDOW..." << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // https://sourceforge.net/p/predef/wiki/OperatingSystems/
    #if defined(__APPLE__) || defined(__MACH__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(1280, 720, "CPEX - T1", NULL, NULL);
    if (!window) {
        std::cerr << "[GLFW] CREATING WINDOW FAILED!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, on_window_resize);

    auto version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "[GLAD] FAILED TO INIT OPENGL!" << std::endl;
        glfwTerminate();
        return -1;
    }
    else {
        std::cout << "[GLAD] RETURNED VERSION: OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;
    }

    // Debug check
    int contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

    if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "[GLFW] DEBUG CONTEXT ACTIVATED!" << std::endl;
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // Set render viewport size
    int winWid = -1, winHei = -1;
    glfwGetWindowSize(window, &winWid, &winHei);
    if (winWid > 0 && winHei > 0) {
        glViewport(0, 0, winWid, winHei);
    }
    else {
        std::cerr << "INVALID WINDOW SIZE: (" << winWid << ", " << winHei << ")" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup scene
    // (model)
    gfx::Vb<gfx::VertPosUv> vb;
    
    vb.set_format(gfx::VertFormat {
        gfx::VertAttribute(0, 3, GL_DOUBLE, sizeof(double), 5, 0), // POS
        gfx::VertAttribute(1, 2, GL_DOUBLE, sizeof(double), 5, 3), // UV
    });
    vb.push_back_verts(std::vector<gfx::VertPosUv> {
        gfx::VertPosUv(-0.5, -0.5, 0.0, 0.0, 0.0),
        gfx::VertPosUv(0.5, -0.5, 0.0, 1.0, 0.0),
        gfx::VertPosUv(-0.5, 0.5, 0.0, 0.0, 1.0),
        gfx::VertPosUv(0.5, 0.5, 0.0, 1.0, 1.0),
    });
    vb.push_back_indices({
        0, 1, 2,
        1, 2, 3,
    });
    vb.build();

    // (shaders)
    gfx::Shader shd;
    shd = gfx::Shader("triangle");
    
    try {
        shd.load_shader_from("./data/triangle.vert", GL_VERTEX_SHADER);
        shd.load_shader_from("./data/triangle.frag", GL_FRAGMENT_SHADER);
        shd.link_program();
    }
    catch (std::runtime_error err) {
        std::cerr << "FAILED TO PREPARE SHADER!" << std::endl << err.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    // Begin loop
    glClearColor(1.0f, 0.8f, 0.25f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        // Input
        process_input(window);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw VAO
        shd.use_shader();
        vb.submit();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "[GLFW] TERMINATING!" << std::endl;
    glfwTerminate();
    return 0;
}