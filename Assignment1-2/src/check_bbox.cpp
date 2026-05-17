#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model.h"
#include <iostream>

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dummy", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Model m("../models/trophy table.obj");
    const auto& meshes = m.getMeshes();
    if(meshes.empty()) { std::cout << "No meshes\n"; return 0; }
    glm::vec3 minV = meshes[0].vertices[0].Position;
    glm::vec3 maxV = meshes[0].vertices[0].Position;
    for(const auto& mesh : meshes) {
        std::cout << "Mesh: " << mesh.name << " vertices: " << mesh.vertices.size() << "\n";
        for(const auto& v : mesh.vertices) {
            minV = glm::min(minV, v.Position);
            maxV = glm::max(maxV, v.Position);
        }
    }
    std::cout << "BBox Min: " << minV.x << ", " << minV.y << ", " << minV.z << "\n";
    std::cout << "BBox Max: " << maxV.x << ", " << maxV.y << ", " << maxV.z << "\n";
    return 0;
}
