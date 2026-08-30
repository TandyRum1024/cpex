/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */
#include <iostream>

// OpenGL: GLFW
#include <GLFW/glfw3.h>

int main() {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cerr << "[GLFW] GLFW INIT FAILED!" << std::endl;
        return -1;
    }

    std::cout << "[GLFW] CREATING WINDOW..." << std::endl;
    window = glfwCreateWindow(1280, 720, "HELLO WINDOW", NULL, NULL);

    if (!window) {
        std::cerr << "[GLFW] CREATING WINDOW FAILED!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "[GLFW] TERMINATING!" << std::endl;
    glfwTerminate();
    return 0;
}