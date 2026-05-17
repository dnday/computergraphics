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
#include <algorithm>
#include <vector>
#include <cctype>
#include <cmath>

// --- Material struct ---
struct Material {
    glm::vec3 baseColor;
    float     ambient;
    float     diffuse;
    float     specular;
    float     shininess;
    float     alpha;
};

// --- MVP variation states ---
static int  g_projMode = 0;
static int  g_cameraPreset = 0;
static bool g_autoRotate = false;
static bool g_useTexture = true;
static float g_rotateAngle = 0.0f;

#ifndef APP_SOURCE_DIR
#define APP_SOURCE_DIR "."
#endif

const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

static unsigned int loadTexture(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if      (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 2) format = GL_RG;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Texture loaded: " << path << " (" << width << "x" << height << ", " << nrChannels << " ch)" << std::endl;
    } else {
        std::cerr << "ERROR::TEXTURE::FAILED_TO_LOAD: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}

// Helper to convert lowercase
std::string toLower(std::string s) {
    for(char &c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// Material assignment based on mesh name
Material getMaterialForMesh(const std::string& name, int index) {
    std::string lowerName = toLower(name);
    // Add logic if name empty or unhelpful, use index.
    if (lowerName.find("glass") != std::string::npos || lowerName.find("kaca") != std::string::npos) {
        return {glm::vec3(0.45f, 0.34f, 0.62f), 0.02f, 0.18f, 1.05f, 256.0f, 0.35f};
    } else if (lowerName.find("frame") != std::string::npos || lowerName.find("metal") != std::string::npos || lowerName.find("bingkai") != std::string::npos) {
        return {glm::vec3(0.36f, 0.18f, 0.14f), 0.18f, 0.62f, 0.35f, 72.0f, 1.0f};
    } else if (lowerName.find("katana") != std::string::npos || lowerName.find("wep") != std::string::npos || lowerName.find("sword") != std::string::npos || lowerName.find("dagger") != std::string::npos || lowerName.find("pedang") != std::string::npos || lowerName.find("object") != std::string::npos || lowerName.find("item") != std::string::npos) {
        return {glm::vec3(0.98f, 0.94f, 0.88f), 0.46f, 0.92f, 1.35f, 230.0f, 1.0f};
    } else if (lowerName.find("floor") != std::string::npos || lowerName.find("base") != std::string::npos || lowerName.find("plane") != std::string::npos || lowerName.find("meja") != std::string::npos || lowerName.find("table") != std::string::npos) {
        return {glm::vec3(0.34f, 0.17f, 0.11f), 0.24f, 0.66f, 0.24f, 48.0f, 1.0f};
    }
    
    if (index == 2) { 
        // Example fallback, not strictly needed
    }

    // Default fallback
    return {glm::vec3(0.50f, 0.45f, 0.55f), 0.05f, 0.50f, 0.40f, 32.0f, 1.0f};
}

static int getObjectKindForMesh(const std::string& name) {
    std::string lowerName = toLower(name);
    if (lowerName.find("katana") != std::string::npos ||
        lowerName.find("wep") != std::string::npos ||
        lowerName.find("sword") != std::string::npos ||
        lowerName.find("dagger") != std::string::npos ||
        lowerName.find("pedang") != std::string::npos) {
        return 2;
    }
    if (lowerName.find("table") != std::string::npos ||
        lowerName.find("meja") != std::string::npos ||
        lowerName.find("trophy") != std::string::npos) {
        return 3;
    }
    return 0;
}

static void setMaterial(const Shader& shader, const Material& mat) {
    shader.setVec3("material.baseColor",  mat.baseColor);
    shader.setFloat("material.ambient",   mat.ambient);
    shader.setFloat("material.diffuse",   mat.diffuse);
    shader.setFloat("material.specular",  mat.specular);
    shader.setFloat("material.shininess", mat.shininess);
    shader.setFloat("material.alpha",     mat.alpha);
}

static void setLights(const Shader& shader, float time) {
    float keyOrbit = time * 0.36f;
    float fillOrbit = time * 0.24f + 2.1f;
    float pulse = 0.5f + 0.5f * std::sin(time * 1.05f);

    shader.setVec3("lights[0].position", glm::vec3(std::sin(keyOrbit) * 3.2f - 0.8f, 3.0f + std::sin(time * 0.85f) * 0.20f, std::cos(keyOrbit) * 2.6f + 2.4f));
    shader.setVec3("lights[0].color", glm::vec3(1.00f, 0.82f, 0.62f));
    shader.setFloat("lights[0].intensity", 1.65f + pulse * 0.30f);

    shader.setVec3("lights[1].position", glm::vec3(std::sin(fillOrbit) * 4.0f, 1.8f, std::cos(fillOrbit) * 3.6f));
    shader.setVec3("lights[1].color", glm::vec3(0.20f, 0.32f, 0.86f));
    shader.setFloat("lights[1].intensity", 0.42f + (1.0f - pulse) * 0.18f);

    shader.setVec3("lights[2].position", glm::vec3(std::sin(time * 0.70f) * 2.5f, 2.2f, -2.8f + std::cos(time * 0.55f) * 0.7f));
    shader.setVec3("lights[2].color", glm::vec3(0.52f, 0.74f, 1.00f));
    shader.setFloat("lights[2].intensity", 0.92f + std::sin(time * 1.25f) * 0.20f);
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_P:
                g_projMode = 1 - g_projMode;
                std::cout << "Projection: " << (g_projMode == 0 ? "Perspective" : "Orthographic") << std::endl;
                break;
            case GLFW_KEY_V:
                g_cameraPreset = (g_cameraPreset + 1) % 3;
                {
                    const char* names[] = { "Cinematic", "Side", "Top" };
                    std::cout << "Camera: " << names[g_cameraPreset] << std::endl;
                }
                break;
            case GLFW_KEY_R:
                g_autoRotate = !g_autoRotate;
                std::cout << "Auto-Rotate: " << (g_autoRotate ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_T:
                g_useTexture = !g_useTexture;
                std::cout << "Texture: " << (g_useTexture ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
            default: break;
        }
    }
}

static std::string buildPath(const std::string& relativePath) {
    return std::string(APP_SOURCE_DIR) + "/" + relativePath;
}

static glm::mat4 buildPresetView(int preset) {
    switch (preset) {
        case 0: // Cinematic angle
            return glm::lookAt(
                glm::vec3(0.35f, 1.55f, 5.10f),
                glm::vec3(0.0f, 0.04f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        case 1: // Side view
            return glm::lookAt(
                glm::vec3(5.7f, 2.1f, 0.0f),
                glm::vec3(0.0f, 0.65f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        case 2: // Top view
            return glm::lookAt(
                glm::vec3(0.0f, 8.0f, 0.0f),
                glm::vec3(0.0f, 0.55f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f)
            );
        default:
            return glm::mat4(1.0f);
    }
}

static unsigned int createFloorVAO(unsigned int& floorVBO) {
    const float y = -0.004f;
    float vertices[] = {
        -24.0f, y, -24.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
         24.0f, y, -24.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         24.0f, y,  24.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -24.0f, y, -24.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
         24.0f, y,  24.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -24.0f, y,  24.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    unsigned int floorVAO = 0;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glBindVertexArray(0);
    return floorVAO;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "ERROR::GLFW::INIT_FAILED" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Assignment 1-2 - Katana", nullptr, nullptr);
    if (!window) {
        std::cerr << "ERROR::GLFW::WINDOW_CREATION_FAILED" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

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

    Model showcase(buildPath("models/katana/katana.obj"));

    unsigned int diffuseMap  = loadTexture(buildPath("textures/katana/katana_diffuse.jpg"));
    unsigned int specularMap = loadTexture(buildPath("textures/katana/katana_specular.jpg"));
    unsigned int roughnessMap = loadTexture(buildPath("textures/katana/katana_metallic.jpg"));
    unsigned int alphaMap    = diffuseMap;
    unsigned int floorVBO = 0;
    unsigned int floorVAO = createFloorVAO(floorVBO);

    shader.use();
    shader.setInt("texDiffuse",  0);
    shader.setInt("texSpecular", 1);
    shader.setInt("texRoughness", 2);
    shader.setInt("texAlpha",    3);

    float lastFrame = 0.0f;

    std::cout << "Controls:\n"
              << "  T    : Toggle Texture / Material color mode\n"
              << "  P    : Toggle Perspective / Orthographic projection\n"
              << "  V    : Cycle camera preset (Cinematic / Side / Top)\n"
              << "  R    : Toggle auto-rotate model\n"
              << "  ESC  : Quit\n";

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime    = currentFrame - lastFrame;
        lastFrame          = currentFrame;

        if (g_autoRotate)
            g_rotateAngle += 45.0f * deltaTime;

        // Cinematic background
        glClearColor(0.010f, 0.012f, 0.024f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 showcaseModel = glm::mat4(1.0f);
        if (g_autoRotate)
            showcaseModel = glm::rotate(showcaseModel, glm::radians(g_rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        showcaseModel = glm::rotate(showcaseModel, glm::radians(-24.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        showcaseModel = glm::rotate(showcaseModel, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        showcaseModel = glm::scale(showcaseModel, glm::vec3(0.060f));
        showcaseModel = glm::translate(showcaseModel, glm::vec3(0.0f, -26.8844f, -5.3683f));

        glm::mat4 view = buildPresetView(g_cameraPreset);
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 viewPos = glm::vec3(invView[3]);

        float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
        glm::mat4 projection;
        if (g_projMode == 0) {
            projection = glm::perspective(glm::radians(33.0f), aspect, 0.1f, 100.0f);
        } else {
            float orthoSize = 3.7f;
            projection = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
        }

        shader.setMat4("view",       view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos",    viewPos);
        shader.setVec2("screenSize", static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT));

        setLights(shader, currentFrame);

        Material floorMat { glm::vec3(0.018f, 0.022f, 0.040f), 0.18f, 0.10f, 0.0f, 32.0f, 1.0f };
        setMaterial(shader, floorMat);
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setInt("objectKind", 1);
        shader.setInt("renderPass", 0);
        shader.setBool("useTexture", false);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        shader.setBool("useTexture", g_useTexture);
        if (g_useTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, roughnessMap);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, alphaMap);
        }

        const auto& meshes = showcase.getMeshes();
        shader.setMat4("model", showcaseModel);
        shader.setInt("objectKind", 0);

        // 1. Render Opaque pass
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        shader.setInt("renderPass", 0);
        
        for (size_t i = 0; i < meshes.size(); ++i) {
            Material mat = getMaterialForMesh(meshes[i].name, i);
            setMaterial(shader, mat);
            shader.setInt("objectKind", getObjectKindForMesh(meshes[i].name));
            meshes[i].Draw(shader);
        }

        // 2. Render Transparent pass (Glass)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Don't write to depth buffer for transparent parts
        shader.setInt("renderPass", 1);
        
        for (size_t i = 0; i < meshes.size(); ++i) {
            Material mat = getMaterialForMesh(meshes[i].name, i);
            setMaterial(shader, mat);
            shader.setInt("objectKind", getObjectKindForMesh(meshes[i].name));
            meshes[i].Draw(shader);
        }

        // Restore state
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
