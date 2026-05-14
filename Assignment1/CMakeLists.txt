cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

project(OpenGLAssignment LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

add_executable(
    app
    src/main.cpp
    src/glad.c
    src/model.cpp
    src/mesh.cpp
    src/stb_image.cpp
)

target_include_directories(
    app
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/headers
)

target_compile_definitions(
    app
    PRIVATE
    APP_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(
    app
    PRIVATE
    glfw
    assimp
    dl
    OpenGL::GL
)
