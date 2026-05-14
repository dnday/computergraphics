#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "model.h"
#include "shader.h"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#ifndef APP_SOURCE_DIR
#define APP_SOURCE_DIR "."
#endif

const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera state
float orbitYaw      = 45.0f;
float orbitPitch    = 28.0f;
float orbitRadius   = 8.0f;
glm::vec3 orbitTarget = glm::vec3(0.0f, 0.8f, 0.0f);
bool  mouseDown     = false;
bool  firstMouse    = true;
float lastX = 0, lastY = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    orbitRadius -= (float)yoffset * 0.35f;
    orbitRadius  = glm::clamp(orbitRadius, 1.5f, 25.0f);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseDown  = (action == GLFW_PRESS);
        firstMouse = true;
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseDown) return;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; return; }
    orbitYaw   += (xpos - lastX) * 0.35f;
    orbitPitch -= (ypos - lastY) * 0.35f;
    orbitPitch  = glm::clamp(orbitPitch, 5.0f, 82.0f);
    lastX = xpos; lastY = ypos;
}

static std::string buildPath(const std::string& relativePath) {
    return std::string(APP_SOURCE_DIR) + "/" + relativePath;
}

static unsigned int loadTexture(const std::string& path) {
    unsigned int textureID; glGenTextures(1, &textureID);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    stbi_image_free(data); return textureID;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cinematic Trophy Case", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    Shader shader(buildPath("shaders/vertex.glsl").c_str(), buildPath("shaders/fragment.glsl").c_str());
    Model showcase(buildPath("models/trophy table.obj"));

    unsigned int texDiffuse = loadTexture(buildPath("textures/trophytable1_texture_(3).png"));
    shader.use();
    shader.setInt("texture_diffuse1", 0);

    while (!glfwWindowShouldClose(window)) {
        float t = glfwGetTime();

        // Dark navy background with slight blue tint
        glClearColor(0.055f, 0.055f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Camera calculations
        float pr = glm::radians(orbitPitch);
        float yr = glm::radians(orbitYaw);
        glm::vec3 cameraPos = orbitTarget + glm::vec3(
            orbitRadius * cos(pr) * sin(yr),
            orbitRadius * sin(pr),
            orbitRadius * cos(pr) * cos(yr)
        );
        glm::mat4 view = glm::lookAt(cameraPos, orbitTarget, glm::vec3(0,1,0));
        glm::mat4 projection = glm::perspective(glm::radians(42.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos", cameraPos);

        // Light Setup
        glm::vec3 keyPos(sin(t * 0.5f) * 7.0f, 9.0f, cos(t * 0.5f) * 7.0f);
        shader.setVec3 ("lights[0].position",  keyPos);
        shader.setVec3 ("lights[0].color",     glm::vec3(1.00f, 0.92f, 0.76f));
        shader.setFloat("lights[0].intensity", 5.0f);

        shader.setVec3 ("lights[1].position",  glm::vec3(-5.0f, 4.0f, 3.0f));
        shader.setVec3 ("lights[1].color",     glm::vec3(0.28f, 0.35f, 0.72f));
        shader.setFloat("lights[1].intensity", 2.0f);

        shader.setVec3 ("lights[2].position",  glm::vec3(0.0f, 7.0f, -6.0f));
        shader.setVec3 ("lights[2].color",     glm::vec3(0.55f, 0.42f, 0.88f));
        shader.setFloat("lights[2].intensity", 1.5f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texDiffuse);

        // Model Matrix Setup
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, -showcase.bbCenter * showcase.autoScale);
        modelMat = glm::scale(modelMat, glm::vec3(showcase.autoScale));
        modelMat = glm::rotate(modelMat, glm::radians((float)glfwGetTime() * 8.0f), glm::vec3(0, 1, 0));
        shader.setMat4("model", modelMat);

        // ── PASS 1: All opaque meshes ──────────────────
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        showcase.DrawOpaque(shader);

        // ── PASS 2: Glass last (transparency) ──────────
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        showcase.DrawTransparent(shader);

        // ── Restore ────────────────────────────────────
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate(); return 0;
}
