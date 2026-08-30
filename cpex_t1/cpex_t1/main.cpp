/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */
#include <iostream>

// OpenGL: GLAD
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
// OpenGL: GLFW
#include <GLFW/glfw3.h>

void on_window_resize(GLFWwindow* win, int wid, int hei) {
    glViewport(0, 0, wid, hei);
    std::cout << "NEW WINDOW SIZE: (" << wid << ", " << hei << ")" << std::endl;
}

int main() {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cerr << "[GLFW] GLFW INIT FAILED!" << std::endl;
        return -1;
    }

    std::cout << "[GLFW] CREATING WINDOW..." << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // https://sourceforge.net/p/predef/wiki/OperatingSystems/
    #ifdef __APPLE__ || __MACH__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(1280, 720, "HELLO WINDOW", NULL, NULL);
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

    // Begin loop
    glClearColor(1.0, 0.8, 0.25, 1.0);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "[GLFW] TERMINATING!" << std::endl;
    glfwTerminate();
    return 0;
}