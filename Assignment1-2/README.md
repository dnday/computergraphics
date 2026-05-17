

# OpenGL 3D Showcase

This project is a modern OpenGL 3D viewer application created for a university computer graphics assignment. It demonstrates core concepts of the modern graphics pipeline, including model loading, transformations, lighting, and GPU-accelerated rendering.

## Features

*   **Modern OpenGL 3.3 Core:** Utilizes a programmable pipeline with shaders.
*   **C++17:** Written in modern C++.
*   **CMake Build System:** Cross-platform build configuration.
*   **Model Loading:** Loads 3D models using the Assimp library.
*   **Phong Lighting:** Implements the classic ambient, diffuse, and specular lighting model.
*   **Camera System:** A fly-through camera for scene navigation.
*   **GPU Accelerated:** Configured for hardware-accelerated rendering, including support for NVIDIA Optimus/PRIME on Linux.

## Project Structure

```
.
├── CMakeLists.txt
├── headers
│   ├── camera.h
│   ├── mesh.h
│   ├── model.h
│   └── shader.h
├── models
│   └── showcase-tables.obj
├── shaders
│   ├── fragment.glsl
│   └── vertex.glsl
├── src
│   ├── glad.c
│   ├── main.cpp
│   ├── mesh.cpp
│   └── model.cpp
└── textures
    ├── Glass.png
    ├── Metal.png
    └── Wood.png
```

## Dependencies

*   GCC / G++ (C++17 compatible)
*   CMake (3.10+)
*   GLFW3
*   GLM
*   Assimp
*   OpenGL 3.3+ drivers

### Installing Dependencies on Ubuntu

```bash
sudo apt update
sudo apt install build-essential cmake git gdb
sudo apt install libglfw3-dev libglm-dev libassimp-dev mesa-utils
```

## How to Build and Run

1.  **Clone the repository and navigate to the project directory.**

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project with CMake:**
    ```bash
    cmake ..
    ```

4.  **Compile the project:**
    ```bash
    make
    ```

5.  **Run the application:**
    ```bash
    ./app
    ```

## GPU Acceleration on Linux (NVIDIA Hybrid Graphics)

To ensure the application uses the dedicated NVIDIA GPU instead of the integrated one or a software renderer like `llvmpipe`, you can use one of the following methods.

### Verification

First, check which renderer is being used by default:
```bash
glxinfo | grep "OpenGL renderer"
```
If the output shows "llvmpipe" or your integrated GPU (e.g., "Intel"), you need to explicitly request the dedicated GPU.

### Method 1: Using `prime-run`

If you have `nvidia-prime` installed, you can launch the application with `prime-run`:
```bash
prime-run ./app
```

### Method 2: Using Environment Variables

You can set environment variables to instruct the loader to use the NVIDIA driver. This is useful for configuring IDEs like VSCode.
```bash
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./app
```

### VSCode `launch.json` for GPU Offloading

To automate this in VSCode, create a `.vscode/launch.json` file with the following configuration. This will set the required environment variables when you launch the debugger.

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug (NVIDIA GPU)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/app",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build",
            "environment": [
                {
                    "name": "__NV_PRIME_RENDER_OFFLOAD",
                    "value": "1"
                },
                {
                    "name": "__GLX_VENDOR_LIBRARY_NAME",
                    "value": "nvidia"
                },
                {
                    "name": "DISPLAY",
                    "value": ":0"
                }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: build"
        }
    ]
}
```

## Core Graphics Concepts Explained

### Modern Graphics Pipeline

The modern graphics pipeline is a series of stages that take your 3D data and transform it into the final 2D image on your screen. Unlike the old fixed-function pipeline, key stages are now programmable using small programs called **shaders**.

1.  **Vertex Data:** The pipeline starts with raw vertex data (positions, colors, normals, texture coordinates) stored in memory.
2.  **CPU to GPU Data Flow:** We send this data from the CPU's RAM to the GPU's VRAM by creating **Vertex Buffer Objects (VBOs)**. This is a one-time (or infrequent) transfer, making it very efficient.
3.  **Vertex Shader:** A program that runs on the GPU for *each vertex*. Its main job is to transform vertex positions from model space into clip space. It also passes data (like normals and texture coordinates) down the pipeline.
4.  **Primitive Assembly:** The GPU connects the transformed vertices into primitives (triangles, lines, points).
5.  **Geometry Shader (Optional):** Can create new primitives or modify existing ones on the fly.
6.  **Rasterization:** The GPU figures out which pixels on the screen are covered by each primitive. This process converts the vector shapes into a "raster" of fragments (potential pixels).
7.  **Fragment Shader:** A program that runs for *each fragment* generated by the rasterizer. Its primary job is to calculate the final color of the fragment. This is where lighting, texturing, and other effects are applied.
8.  **Tests and Blending:** Before a fragment becomes a pixel, it goes through tests like the **depth test** (which prevents objects behind others from being drawn) and **stencil test**. Blending combines the fragment's color with the existing color in the framebuffer.

### VAO, VBO, and EBO Roles

*   **VBO (Vertex Buffer Object):** A memory buffer on the GPU that stores vertex data (positions, normals, etc.). It's the raw data.
*   **VAO (Vertex Array Object):** Acts as a "configuration" or "state" object. It stores the setup for how to interpret the vertex data from one or more VBOs. This includes:
    *   Which VBOs to use.
    *   How the vertex attributes (position, normal, etc.) are laid out in the VBO's memory (e.g., "the position is 3 floats, starting at offset 0").
    *   Which EBO is active.
    By binding a VAO, you instantly apply a whole set of data and configuration, which is much more efficient than setting it up every time you draw.
*   **EBO (Element Buffer Object):** An optional buffer that stores indices. It allows you to define a set of unique vertices and then specify the order in which to draw them to form triangles. This avoids duplicating vertex data and saves memory.

### Phong Lighting Model

The Phong model is a simple but effective way to simulate how light interacts with a surface. It's calculated in the fragment shader and has three components:

1.  **Ambient:** Simulates indirect light that's been scattered around the scene. It gives a base color to objects so they are never completely black. It's a simple multiplication: `ambient = lightColor * objectColor * ambientStrength`.
2.  **Diffuse:** Simulates the directional impact of a light source on a surface. The more a surface faces the light source, the brighter it becomes. It depends on the angle between the surface normal and the light direction.
3.  **Specular:** Simulates the bright, shiny highlight that appears on smooth surfaces. It depends on the angle between the view direction and the reflected light direction. The `shininess` value controls the size and intensity of the highlight.

The final color is the sum of these three components.
