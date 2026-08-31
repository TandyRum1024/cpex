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

# Setup list of external libraries to be automatically used for `target_link_libraries()`
set(EXT_LIBS)

# Setup list of sources to be automatically used for `add_executable()`
set(SRC_LIBS)
set(SRC_LIBS_INCL)

# Alias for source files
set(
    SRC_MAIN
    cpex_t1/main.cpp
)
# (GFX)
set(DIR_GFX ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/gfx)
list(
    APPEND
    SRC_LIBS
    ${DIR_GFX}/shader.cpp
    ${DIR_GFX}/vb.cpp
    ${DIR_GFX}/vert.cpp
)
# (ZCL)
set(DIR_ZCL ${CMAKE_CURRENT_SOURCE_DIR}/zcl)
list(
    APPEND
    SRC_LIBS
    ${DIR_ZCL}/zcl.cpp
)
# (GLAD - https://gen.glad.sh/)
set(DIR_GLAD ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad)
list(
    APPEND
    SRC_LIBS
    ${DIR_GLAD}/src/gl.c
)

list(
    APPEND
    SRC_LIBS_INCL
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1 # GFX, etc
    ${DIR_ZCL}/include   
    ${DIR_GLAD}/include
)

# For building `cpex_t1.exe` with source files `<root>/cpex_t1/main.cpp`
include_directories(${SRC_LIBS_INCL})
add_executable(${PROJECT_NAME} ${SRC_MAIN} ${SRC_LIBS})

# Shaders & misc. files copy, using `copy_directory_if_different`
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
# target_link_libraries(${PROJECT_NAME} OpenGL::GL) # `${OPENGL_LIBRARIES}` == legacy
list(APPEND EXT_LIBS OpenGL::GL) # `${OPENGL_LIBRARIES}` == legacy

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
list(APPEND EXT_LIBS glfw)

# (LEGACY: USE SUBMODULES INSTEAD)
# add_subdirectory(${SRC_GLFW})
# -----------------------
# External dependencies

# Link external dependencies with list built above
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

# Testing (maybe)
include(CTest)
enable_testing()
