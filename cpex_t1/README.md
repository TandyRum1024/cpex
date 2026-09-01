# CPEX - T1, C++ (re)primer and OpenGL basics
---
First C++ project in a while, so this project focuses on (re)priming my C++ knowledge and using OpenGL and other external dependencies to do fun stuffs.

# Building
Currently this project is being written on Windows machine with Microsoft Visual Studio 2022, with x64 target in mind.
Please refer to [this link](https://code.visualstudio.com/docs/cpp/config-msvc) for downloading and setting up the environment.

For sake of learning, I've been trying out CMake. Specifically, [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) from VSCode.
Therefore, if possible, I recommend doing the same.

The `CPEX_x64 (Visual Studio Community 2022 Release - amd64)` configure preset is current being used.

Building on VSCode environment is as simple as... (on command palette, activated by `CTRL + SHIFT + P`)
- CMake: Run without debugging (`CTRL + SHIFT + F5`)
- CMake: Build (`F7`)
- CMake: Debug (`SHIFT + F5`)

Alternatively, you may generate, build and run manually. On CMake Tools, I've set up the build path as `"<root>/out/build/CPEX_x64"`. Therefore you may manually build into the same directory and run it like the snippet below.
```powershell
# (ASSUMING YOUR WORKING DIRECTORIES ARE AT THE PROEJCT ROOT)
# cd <THIS DIRECTORY>

# (generate build files)
cmake -G "Visual Studio 17 2022" -B ./out/build/CPEX_x64 -T host=x64 -A x64

# (build)
cmake --build ./out/build/CPEX_x64 --config Debug # Or --config Release

# (run)
./out/build/CPEX_x64/Debug/cpex_t1.exe # Directory will be different if you are using `Release` config

# (clean)
cmake --build ./out/build/CPEX_x64 --config Debug --target clean
```

# (expected) Dependencies
- OpenGL
    - [GLFW 3.5.1](https://www.glfw.org/)
    - [GLAD2](https://gen.glad.sh/)
- [Dear ImGui](https://github.com/ocornut/imgui)