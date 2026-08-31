# CPEX - Build management madness
# Written with duct tape, hope and dreams
# ZIK@MMXXVI
cmake_minimum_required(VERSION 3.26.0)
project(
    cpex_t1
    VERSION 0.1.0
    DESCRIPTION "CPP EXPERIMENTS: TOY #1"
    LANGUAGES CXX
)

# Using `FetchContent` for better external library handling! wow...
# https://www.foonathan.net/2022/06/cmake-fetchcontent/
include(FetchContent)
# set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/extern/build")

# Alias for source files
set(
    SRC_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/gfx.cpp
)
set(
    SRC_LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/zcl
)
# (GLAD - https://gen.glad.sh/)
set(
    SRC_GLAD
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad
)

set(
    SRC_INCL
    ${SRC_LIBS}/include   
    ${SRC_GLAD}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1
)

# For building `cpex_t1.exe` with source files `<root>/cpex_t1/main.cpp`
include_directories(${SRC_INCL})
add_executable(${PROJECT_NAME} ${SRC_MAIN} ${SRC_LIBS}/zcl.cpp ${SRC_GLAD}/src/gl.c)

# Shaders & misc. files using `copy_directory_if_different`
# https://discourse.cmake.org/t/copying-files-with-destination-folder-depending-on-build-config/10686
add_custom_target(
    COPY_DATA
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/data"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
)
add_dependencies(${PROJECT_NAME} COPY_DATA)

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
