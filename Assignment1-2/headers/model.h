#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

#include "mesh.h"

class Model {
public:
    explicit Model(const std::string& path);
    void Draw(const Shader& shader) const;
    const std::vector<Mesh>& getMeshes() const { return meshes; }

private:
    std::vector<Mesh> meshes;

    void loadModel(const std::string& path);
    void processNode(struct aiNode* node, const struct aiScene* scene);
    Mesh processMesh(struct aiMesh* mesh, const struct aiScene* scene);
};

#endif
