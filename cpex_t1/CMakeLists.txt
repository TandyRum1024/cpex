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

# Set C++ standards
set(CMAKE_CXX_STANDARD 20)

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
    cpex_t1/app.cpp
)
# (GFX)
set(DIR_GFX ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/gfx)
list(
    APPEND
    SRC_LIBS
    ${DIR_GFX}/material.cpp
    ${DIR_GFX}/shader.cpp
    ${DIR_GFX}/uniform.cpp
    ${DIR_GFX}/vb.cpp
    ${DIR_GFX}/vert.cpp
)
# (ZAP)
set(DIR_ZAP ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/zap)
list(
    APPEND
    SRC_LIBS
    ${DIR_ZAP}/zap.cpp
    ${DIR_ZAP}/opengl/opengl.cpp
)
# (ZCL)
set(DIR_ZCL ${CMAKE_CURRENT_SOURCE_DIR}/zcl)
list(
    APPEND
    SRC_LIBS
    ${DIR_ZCL}/zcl.cpp
)

list(
    APPEND
    SRC_LIBS_INCL
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1 # GFX, etc
    ${DIR_ZCL}/include
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

# (GLAD - https://gen.glad.sh/)
set(DIR_GLAD ${CMAKE_CURRENT_SOURCE_DIR}/../common_extern/glad)
target_sources(
    ${PROJECT_NAME}
    PUBLIC
    ${DIR_GLAD}/src/gl.c
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${DIR_GLAD}/include
)

# Dear ImGui (https://github.com/ocornut/imgui)
set(DIR_IMGUI ${CMAKE_CURRENT_SOURCE_DIR}/../common_extern/imgui)
target_sources(
    ${PROJECT_NAME}
    PUBLIC
    # (CORE)
    ${DIR_IMGUI}/imgui.cpp
    ${DIR_IMGUI}/imgui_demo.cpp
    ${DIR_IMGUI}/imgui_draw.cpp
    ${DIR_IMGUI}/imgui_tables.cpp
    ${DIR_IMGUI}/imgui_widgets.cpp
    # (BACKEND)
    ${DIR_IMGUI}/backends/imgui_impl_glfw.cpp
    ${DIR_IMGUI}/backends/imgui_impl_opengl3.cpp
    # (MISC)
    ${DIR_IMGUI}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${DIR_IMGUI}
    ${DIR_IMGUI}/misc/cpp
    ${DIR_IMGUI}/backends
)

# spdlog (https://github.com/gabime/spdlog)
set(VER_SPDLOG "v1.17.0")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG ${VER_SPDLOG}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)
list(APPEND EXT_LIBS spdlog)

# GLM (https://github.com/g-truc/glm)
set(VER_GLM "1.0.3")
FetchContent_Declare(
    glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG ${VER_GLM}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)
list(APPEND EXT_LIBS glm)

# (STB - https://github.com/nothings/stb)
set(DIR_STB ${CMAKE_CURRENT_SOURCE_DIR}/../common_extern/stb)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${DIR_STB}
)
# -----------------------
# External dependencies

# Link external dependencies with list built above
target_link_libraries(${PROJECT_NAME} ${EXT_LIBS})

# Testing (maybe)
include(CTest)
enable_testing()
