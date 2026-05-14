#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent; 
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string name;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::string name);
    void Draw(const Shader& shader) const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    void setupMesh();
};

#endif
