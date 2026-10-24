# CPEX - T1, C++ (re)primer and OpenGL basics
---
First C++ project in a while, so this project focuses on (re)priming my C++ knowledge and using OpenGL and other external dependencies to do fun stuffs.

# Features
- OpenGL object abstraction (shader, shader uniform, texture, VBO/VAO/EBO)
- Additional classes for easier rendering (material, vertex format)
- Basic OpenGL app framework (`zap`) that manages window and handles rendering/game loop
- (Multi-)textured mesh with uniforms
- ImGUI for adjusting uniforms

# Building
## VSCode `CMake-Tools` extension (Windows x64 MSVC)
Currently this project is being written on Windows machine with Microsoft Visual Studio 2022, with x64 target in mind.
Please refer to [this link](https://code.visualstudio.com/docs/cpp/config-msvc) for downloading and setting up the environment.

For sake of learning, I've been trying out CMake. Specifically, [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) from VSCode.
Therefore, if possible, I recommend doing the same.

The `CPEX_x64 (Visual Studio Community 2022 Release - amd64)` configure preset is current being used.

Building on VSCode environment is as simple as... (on command palette, activated by `CTRL + SHIFT + P`)
- CMake: Run without debugging (`CTRL + SHIFT + F5`)
- CMake: Build (`F7`)
- CMake: Debug (`SHIFT + F5`)

## Cmake (Windows)
Alternatively, you may generate, build and run manually. For Windows machine, I've set up a convenience script, `build.bat`.
```powershell
build.bat --config [Debug|Release] (default: Debug) --architecture x64 (default: x64) [-skip-config] [-norun] [-clean]

# --config - Either Debug or Release, determines the build target.
# --architecture - Currently being written with x64 in mind. No other architectures are tested.
# -skip-config - If set, Skips CMake configuration.
# -norun - The script runs the built executable by default. If set, then it will not run.
# -clean - If set, deletes `./out` folder before running config/build.
```

## Cmake (Linux / Ubuntu)
Similarly, for Linux, I've set up a convenience script, `build.sh`. Do mind that I have only tested this on the `Ubuntu 24.04.5 LTS` environment.

For a successful build on `Ubuntu 24.04.5 LTS`, you may need to install a few dependencies for the project to find & use it:
```bash
# git (for FetchContent, if you somehow forgot to install it until to this point)
(sudo) apt install git

# build-essential (for GCC/G++, compiling C++)
apt install build-essential

# OpenGL
apt install libgl-dev

# wayland (for GLFW to use it)
apt install libwayland-dev libxkbcommon-dev xorg-dev
```

Then, run the build script:
```bash
build.sh --config [Debug|Release] (default: Debug) --architecture [x64|x32] (default: x64) [-skip-config] [-norun] [-clean]

# --config - Either Debug or Release, determines the build target.
# --architecture - Currently being written with x64 in mind. No other architectures are tested.
# -skip-config - If set, Skips CMake configuration.
# -norun - The script runs the built executable by default. If set, then it will not run.
# -clean - If set, deletes `./out` folder before running config/build.
```

# Dependencies
- OpenGL (4.3, Core)
    - [GLFW 3.5.1](https://www.glfw.org/)
    - [GLAD2](https://gen.glad.sh/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [spdlog](https://github.com/gabime/spdlog)
- [GLM](https://github.com/g-truc/glm)
- [stb_image.h](https://github.com/nothings/stb)