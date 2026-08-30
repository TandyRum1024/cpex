# CPEX - Build management madness
# Written with duct tape, hope and dreams
# ZIK@MMXXVI
cmake_minimum_required(VERSION 3.11.0)
project(
    cpex_t1
    VERSION 0.1.0
    DESCRIPTION "CPP EXPERIMENTS: TOY #1"
    LANGUAGES CXX
)

# Using FetchContent for better external library handling! wow...
# https://www.foonathan.net/2022/06/cmake-fetchcontent/
include(FetchContent)
# set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/extern/build")

# Alias for source files
set(
    SRC_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/main.cpp 
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
set(VER_GLFW "3.5.1")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG ${VER_GLFW}
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
# (link program with GLFW)
target_link_libraries(${PROJECT_NAME} glfw)
# (LEGACY: USE SUBMODULES INSTEAD)
# add_subdirectory(${SRC_GLFW})
# -----------------------
# External dependencies

# Testing (maybe)
include(CTest)
enable_testing()
