/**
 * CPEX - T1, C++ (re)primer and OpenGL basics
 * ZIK@MMXXVI
 */
#include <iostream>
#include <fstream>
#include <string>

// LIBRARIES //
#include <gfx.hpp>
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
    #if defined(__APPLE__) || defined(__MACH__)
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

    // Setup scene
    // (model)
    GLuint vao, vboPos, vboUv;
    double positions[] = {
        -0.5, -0.5, 0.0,
        0.0, 0.5, 0.0,
        0.5, -0.5, 0.0,
    };
    double uvs[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
    };

    // Setup VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Setup positions VBO
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*) 0); // (location = 0)
    glEnableVertexAttribArray(0);
    
    // Setup uvs VBO
    glGenBuffers(1, &vboUv);
    glBindBuffer(GL_ARRAY_BUFFER, vboUv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*) 0); // (location = 1)
    glEnableVertexAttribArray(1);

    // (shaders)
    gfx::Shader shd("triangle");
    try {
        shd.load_shader_from("./data/triangle.vert", GL_VERTEX_SHADER);
        shd.load_shader_from("./data/triangle.frag", GL_FRAGMENT_SHADER);
        shd.link_program();

        shd.load_shader_from("./data/triangle_alt.frag", GL_FRAGMENT_SHADER);
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
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(positions) / sizeof(positions[0]));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "[GLFW] TERMINATING!" << std::endl;
    glfwTerminate();
    return 0;
}