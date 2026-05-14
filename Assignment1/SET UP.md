# OpenGL Project Setup Guide

This guide will help you set up and run the **Cinematic Trophy Case** project on your local machine.

## 1. Prerequisites

Ensure you have the following installed:
- **CMake** (v3.10 or higher)
- **C++ Compiler** (GCC/Clang on Linux, MSVC on Windows)
- **Git**
- **GPU Drivers** (Make sure your NVIDIA/AMD drivers are up to date)

### Dependencies
The project requires the following libraries:
- **GLFW3**: Window management and input.
- **Assimp**: 3D model loading.
- **GLM**: OpenGL Mathematics.
- **OpenGL**: Core graphics library.

On Linux (Ubuntu/Debian), you can install them via:
```bash
sudo apt-get update
sudo apt-get install libglfw3-dev libassimp-dev libglm-dev libgles2-mesa-dev
```

## 2. Project Structure

- `headers/` & `src/`: Core logic (Mesh, Model, Shader loading).
- `shaders/`: GLSL Fragment and Vertex shaders.
- `models/`: 3D model files (.obj).
- `textures/`: Image textures.
- `include/`: Third-party headers (GLAD/KHR).

## 3. Building the Project

We use CMake for the build system. Follow these steps:

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Configure the project
cmake ..

# 3. Compile
make -j4
```

## 4. Running the Application

After a successful build, run the executable from the `build` directory:
```bash
./app
```

## 5. Controls

- **ESC**: Close the application.
- **Camera**: The camera is fixed at a cinematic angle to match the reference design.

## 6. GPU Usage Note

The code includes a request for the dedicated GPU (`NvOptimusEnablement` and `AmdPowerXpressRequestHighPerformance`). If you are on a laptop with dual graphics (Integrated + Dedicated), the OS should automatically prefer the high-performance GPU.

## 7. Troubleshooting

- **Shaders not loading**: Ensure you are running the executable from the `build` directory or the project root so the relative paths to `shaders/` are correct.
- **Missing Models**: Check the `models/` folder for `trophy table.obj`.
