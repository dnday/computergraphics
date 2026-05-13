#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "model.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>

#ifndef APP_SOURCE_DIR
#define APP_SOURCE_DIR "."
#endif

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

static std::string buildPath(const std::string& relativePath) {
    return std::string(APP_SOURCE_DIR) + "/" + relativePath;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "ERROR::GLFW::INIT_FAILED" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Assignment", nullptr, nullptr);
    if (!window) {
        std::cerr << "ERROR::GLFW::WINDOW_CREATION_FAILED" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR::GLAD::INIT_FAILED" << std::endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader(
        buildPath("shaders/vertex.glsl").c_str(),
        buildPath("shaders/fragment.glsl").c_str()
    );

    Model showcase(buildPath("models/trophy table.obj"));
    Camera camera;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
            0.1f,
            100.0f
        );

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        shader.setVec3("lightPos", glm::vec3(2.0f, 4.0f, 2.0f));
        shader.setVec3("viewPos", camera.Position);

        showcase.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
