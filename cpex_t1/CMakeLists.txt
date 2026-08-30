cmake_minimum_required(VERSION 3.10.0)
project(
    cpex_t1
    VERSION 0.1.0
    DESCRIPTION "CPP EXPERIMENTS: TOY #1"
    LANGUAGES CXX
)

# Library
# GLFW
find_package(glfw3 3.5 REQUIRED)

# Alias for source files
set(
    SRC_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/cpex_t1/main.cpp
)

# For building `cpex_t1.exe` with source files `<root>/cpex_t1/main.cpp`
add_executable(${PROJECT_NAME} ${SRC_MAIN})

# Link program with GLFW
target_link_libraries(${PROJECT_NAME} glfw)

# Testing (maybe)
include(CTest)
enable_testing()
