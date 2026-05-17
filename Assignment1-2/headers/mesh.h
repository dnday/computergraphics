#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    void Draw(const Shader& shader) const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    void setupMesh();
};

#endif
