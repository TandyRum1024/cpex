cmake_minimum_required(VERSION 3.10.0)
project(
    cpex_t1
    VERSION 0.1.0
    DESCRIPTION "CPP EXPERIMENTS: TOY #1"
    LANGUAGES CXX
)

# Alias for source files
set(
    SRC_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/main.cpp
)

set(
    SRC_GLFW
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw
)

# For building `cpex_t1.exe` with source files `<root>/cpex_t1/main.cpp`
add_executable(${PROJECT_NAME} ${SRC_MAIN})

# External dependencies
# -----------------------
# OpenGL (https://cmake.org/cmake/help/latest/module/FindOpenGL.html)
find_package(OpenGL REQUIRED)
# (link program with OpenGL)
target_link_libraries(${PROJECT_NAME} OpenGL::GL) # `${OPENGL_LIBRARIES}` == legacy

# GLFW (https://www.glfw.org/docs/3.0/build.html#build_link_cmake_source)
add_subdirectory(${SRC_GLFW})
# (link program with GLFW)
target_link_libraries(${PROJECT_NAME} glfw)
# -----------------------
# External dependencies

# Testing (maybe)
include(CTest)
enable_testing()
