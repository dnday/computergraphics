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

// --- Material struct ---
struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     shininess;
};

// 4 preset materials
const Material MAT_GOLD = {
    glm::vec3(0.2473f, 0.1995f, 0.0745f),  // ambient
    glm::vec3(0.7516f, 0.6065f, 0.2265f),  // diffuse
    glm::vec3(0.6283f, 0.5559f, 0.3661f),  // specular
    51.2f                                   // shininess
};

const Material MAT_CHROME = {
    glm::vec3(0.25f, 0.25f, 0.25f),
    glm::vec3(0.40f, 0.40f, 0.40f),
    glm::vec3(0.77f, 0.77f, 0.77f),
    76.8f
};

const Material MAT_RUBBER = {
    glm::vec3(0.05f, 0.05f, 0.05f),
    glm::vec3(0.50f, 0.40f, 0.40f),
    glm::vec3(0.07f, 0.04f, 0.04f),
    10.0f
};

const Material MAT_PLASTIC = {
    glm::vec3(0.10f, 0.10f, 0.30f),
    glm::vec3(0.20f, 0.20f, 0.60f),
    glm::vec3(0.40f, 0.40f, 0.80f),
    32.0f
};

// Active material index (1-based, matches keyboard keys 1-4)
static int g_materialIndex = 1;

// --- MVP variation states ---
// P: projection mode (0 = perspective, 1 = orthographic)
static int  g_projMode = 0;

// V: camera preset index (0 = front, 1 = side, 2 = top)
static int  g_cameraPreset = 0;

// R: auto-rotate model toggle
static bool g_autoRotate = false;

// T: texture toggle (true = use textures, false = use material colors)
static bool g_useTexture = true;

// Rotation angle (degrees), accumulated over time
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

// --- setMaterial: upload Material fields to shader uniforms ---
static void setMaterial(const Shader& shader, const Material& mat) {
    shader.setVec3("material.ambient",    mat.ambient);
    shader.setVec3("material.diffuse",    mat.diffuse);
    shader.setVec3("material.specular",   mat.specular);
    shader.setFloat("material.shininess", mat.shininess);
}

// --- loadTexture: load an image file into an OpenGL texture ---
static unsigned int loadTexture(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if      (nrChannels == 1) format = GL_RED;
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

// --- Keyboard callback ---
void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        switch (key) {
            // Material selection (auto-disables texture mode)
            case GLFW_KEY_1: g_materialIndex = 1; g_useTexture = false; std::cout << "Material: Gold"    << std::endl; break;
            case GLFW_KEY_2: g_materialIndex = 2; g_useTexture = false; std::cout << "Material: Chrome"  << std::endl; break;
            case GLFW_KEY_3: g_materialIndex = 3; g_useTexture = false; std::cout << "Material: Rubber"  << std::endl; break;
            case GLFW_KEY_4: g_materialIndex = 4; g_useTexture = false; std::cout << "Material: Plastic" << std::endl; break;

            // P: toggle projection
            case GLFW_KEY_P:
                g_projMode = 1 - g_projMode;
                std::cout << "Projection: "
                          << (g_projMode == 0 ? "Perspective" : "Orthographic")
                          << std::endl;
                break;

            // V: cycle camera preset
            case GLFW_KEY_V:
                g_cameraPreset = (g_cameraPreset + 1) % 3;
                {
                    const char* names[] = { "Front", "Side", "Top" };
                    std::cout << "Camera: " << names[g_cameraPreset] << std::endl;
                }
                break;

            // R: toggle auto-rotate
            case GLFW_KEY_R:
                g_autoRotate = !g_autoRotate;
                std::cout << "Auto-Rotate: " << (g_autoRotate ? "ON" : "OFF") << std::endl;
                break;

            // T: toggle texture / material mode
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

// --- Build view matrix from camera preset (does NOT use Camera class position) ---
static glm::mat4 buildPresetView(int preset) {
    // All presets look at the world origin from a fixed distance
    switch (preset) {
        case 0: // Front view  — camera at (0, 2, 6), look at origin
            return glm::lookAt(
                glm::vec3(0.0f, 2.0f, 6.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        case 1: // Side view   — camera at (6, 2, 0), look at origin
            return glm::lookAt(
                glm::vec3(6.0f, 2.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        case 2: // Top view    — camera directly above, looking straight down
            return glm::lookAt(
                glm::vec3(0.0f, 8.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f)  // "up" is -Z when looking down Y
            );
        default:
            return glm::mat4(1.0f);
    }
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

    Model showcase(buildPath("models/trophy table.obj"));
    Camera camera;  // kept for its Position field (used when preset == 0)

    // --- Load textures ---
    unsigned int diffuseMap  = loadTexture(buildPath("textures/trophytable1_texture_(3).png"));
    unsigned int specularMap = loadTexture(buildPath("textures/trophytable1_spec_(4).png"));

    // Tell the shader which texture unit each sampler belongs to (only needs to be done once)
    shader.use();
    shader.setInt("texDiffuse",  0);
    shader.setInt("texSpecular", 1);

    // Timing
    float lastFrame = 0.0f;

    std::cout << "Controls:\n"
              << "  1-4  : Switch material (Gold / Chrome / Rubber / Plastic)\n"
              << "  T    : Toggle Texture / Material color mode\n"
              << "  P    : Toggle Perspective / Orthographic projection\n"
              << "  V    : Cycle camera preset (Front / Side / Top)\n"
              << "  R    : Toggle auto-rotate model\n"
              << "  ESC  : Quit\n";

    while (!glfwWindowShouldClose(window)) {
        // --- Delta time ---
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime    = currentFrame - lastFrame;
        lastFrame          = currentFrame;

        // --- Auto-rotate accumulation ---
        if (g_autoRotate)
            g_rotateAngle += 45.0f * deltaTime; // 45 deg/s

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // --- Model matrix ---
        glm::mat4 model = glm::mat4(1.0f);
        if (g_autoRotate)
            model = glm::rotate(model, glm::radians(g_rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

        // --- View matrix ---
        // Preset 0 (Front) uses Camera class position so the default feels natural;
        // presets 1 & 2 use fixed lookAt positions.
        glm::mat4 view;
        glm::vec3 viewPos;
        if (g_cameraPreset == 0) {
            view    = camera.GetViewMatrix();
            viewPos = camera.Position;
        } else {
            view    = buildPresetView(g_cameraPreset);
            // Extract camera world-space position from inverse view for lighting
            glm::mat4 invView = glm::inverse(view);
            viewPos = glm::vec3(invView[3]);
        }

        // --- Projection matrix ---
        float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
        glm::mat4 projection;
        if (g_projMode == 0) {
            // Perspective
            projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        } else {
            // Orthographic — scale chosen so the model fits roughly on screen
            float orthoSize = 5.0f;
            projection = glm::ortho(
                -orthoSize * aspect,  orthoSize * aspect,
                -orthoSize,           orthoSize,
                0.1f, 100.0f
            );
        }

        shader.setMat4("model",      model);
        shader.setMat4("view",       view);
        shader.setMat4("projection", projection);

        shader.setVec3("lightPos", glm::vec3(2.0f, 4.0f, 2.0f));
        shader.setVec3("viewPos",  viewPos);

        // --- Material ---
        const Material* mat = &MAT_GOLD;
        if      (g_materialIndex == 2) mat = &MAT_CHROME;
        else if (g_materialIndex == 3) mat = &MAT_RUBBER;
        else if (g_materialIndex == 4) mat = &MAT_PLASTIC;
        setMaterial(shader, *mat);

        // --- Texture binding ---
        shader.setBool("useTexture", g_useTexture);
        if (g_useTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);
        }

        showcase.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
