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
    for(size_t i=0; i<meshes.size(); ++i) {
        const auto& mesh = meshes[i];
        if(mesh.vertices.empty()) continue;
        glm::vec3 minV = mesh.vertices[0].Position;
        glm::vec3 maxV = mesh.vertices[0].Position;
        for(const auto& v : mesh.vertices) {
            minV = glm::min(minV, v.Position);
            maxV = glm::max(maxV, v.Position);
        }
        std::cout << "Index " << i << " | Name: " << mesh.name 
                  << " | BBox: Min(" << minV.x << "," << minV.y << "," << minV.z 
                  << ") Max(" << maxV.x << "," << maxV.y << "," << maxV.z << ")\n";
    }
    return 0;
}
