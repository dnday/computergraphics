#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "mesh.h"

class Model {
public:
    explicit Model(const std::string& path);
    void Draw(const Shader& shader) const;
    void DrawOpaque(const Shader& shader) const;
    void DrawTransparent(const Shader& shader) const;

    std::vector<Mesh> meshes;
    std::vector<std::string> meshNames;

    glm::vec3 bbMin, bbMax, bbCenter;
    float autoScale;

private:
    void loadModel(const std::string& path);
    void processNode(struct aiNode* node, const struct aiScene* scene);
    Mesh processMesh(struct aiMesh* mesh, const struct aiScene* scene);
    void AssignMaterial(const Shader& shader, const std::string& nameLower, int meshIndex) const;
};

#endif
