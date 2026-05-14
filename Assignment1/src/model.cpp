#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <algorithm>

Model::Model(const std::string& path) : bbMin(1e9f), bbMax(-1e9f), autoScale(1.0f) {
    loadModel(path);

    for (auto& mesh : meshes) {
        for (auto& v : mesh.vertices) {
            bbMin = glm::min(bbMin, v.Position);
            bbMax = glm::max(bbMax, v.Position);
        }
    }
    
    bbCenter = (bbMin + bbMax) * 0.5f;
    float maxDim = glm::max(bbMax.x - bbMin.x, glm::max(bbMax.y - bbMin.y, bbMax.z - bbMin.z));
    autoScale = 3.5f / maxDim;

    std::cout << "\n=== LOADED MESHES ===" << std::endl;
    for (int i = 0; i < meshes.size(); i++) {
        std::cout << "Mesh[" << i << "] name: '" << meshNames[i] << "'" << " | vertices: " << meshes[i].vertices.size() << std::endl;
    }
    std::cout << "=====================\n" << std::endl;
}

void Model::Draw(const Shader& shader) const {
    DrawOpaque(shader);
    DrawTransparent(shader);
}

void Model::DrawOpaque(const Shader& shader) const {
    for (int i = 0; i < meshes.size(); i++) {
        std::string name = meshNames[i];
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        bool isGlass = (name.find("glass") != std::string::npos ||
                        name.find("window") != std::string::npos ||
                        name.find("pane") != std::string::npos ||
                        name.find("crystal") != std::string::npos ||
                        name.find("cube.001") != std::string::npos);
        if (isGlass) continue; 

        AssignMaterial(shader, name, i);
        meshes[i].Draw(shader);
    }
}

void Model::DrawTransparent(const Shader& shader) const {
    for (int i = 0; i < meshes.size(); i++) {
        std::string name = meshNames[i];
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        bool isGlass = (name.find("glass") != std::string::npos ||
                        name.find("window") != std::string::npos ||
                        name.find("pane") != std::string::npos ||
                        name.find("crystal") != std::string::npos ||
                        name.find("cube.001") != std::string::npos);
        if (!isGlass) continue;

        AssignMaterial(shader, name, i);
        meshes[i].Draw(shader);
    }
}

void Model::AssignMaterial(const Shader& shader, const std::string& nameLower, int meshIndex) const {
    shader.setBool("useTexture", false);

    if (nameLower.find("glass") != std::string::npos || nameLower.find("pane") != std::string::npos || nameLower.find("window") != std::string::npos || nameLower.find("crystal") != std::string::npos || nameLower.find("cube.001") != std::string::npos) {
        // GLASS
        shader.setVec3 ("material.baseColor",  glm::vec3(0.58f, 0.44f, 0.72f));
        shader.setFloat("material.ambientStr",  0.35f);
        shader.setFloat("material.diffuseStr",  0.35f);
        shader.setFloat("material.specularStr", 2.80f);
        shader.setFloat("material.shininess",   512.0f);
        shader.setFloat("material.alpha",       0.40f); 
    } else if (nameLower.find("frame") != std::string::npos || nameLower.find("leg") != std::string::npos || nameLower.find("metal") != std::string::npos || nameLower.find("rail") != std::string::npos || nameLower.find("struct") != std::string::npos) {
        // METAL FRAME
        shader.setVec3 ("material.baseColor",  glm::vec3(0.80f, 0.52f, 0.36f));
        shader.setFloat("material.ambientStr",  0.25f);
        shader.setFloat("material.diffuseStr",  0.75f);
        shader.setFloat("material.specularStr", 1.90f);
        shader.setFloat("material.shininess",   192.0f);
        shader.setFloat("material.alpha",       1.0f);
    } else if (nameLower.find("sword") != std::string::npos || nameLower.find("blade") != std::string::npos || nameLower.find("weapon") != std::string::npos || nameLower.find("item") != std::string::npos || nameLower.find("obj") != std::string::npos) {
        // SWORD / ARTIFACT
        shader.setVec3 ("material.baseColor",  glm::vec3(0.95f, 0.76f, 0.18f));
        shader.setFloat("material.ambientStr",  0.45f);
        shader.setFloat("material.diffuseStr",  0.85f);
        shader.setFloat("material.specularStr", 2.20f);
        shader.setFloat("material.shininess",   320.0f);
        shader.setFloat("material.alpha",       1.0f);
    } else if (nameLower.find("shelf") != std::string::npos || nameLower.find("base") != std::string::npos || nameLower.find("floor") != std::string::npos || nameLower.find("bottom") != std::string::npos || nameLower.find("tray") != std::string::npos) {
        // SHELF / BASE
        shader.setVec3 ("material.baseColor",  glm::vec3(0.18f, 0.15f, 0.22f));
        shader.setFloat("material.ambientStr",  0.20f);
        shader.setFloat("material.diffuseStr",  0.45f);
        shader.setFloat("material.specularStr", 0.30f);
        shader.setFloat("material.shininess",   32.0f);
        shader.setFloat("material.alpha",       1.0f);
    } else {
        // FALLBACK
        shader.setVec3 ("material.baseColor",  glm::vec3(0.62f, 0.57f, 0.68f));
        shader.setFloat("material.ambientStr",  0.22f);
        shader.setFloat("material.diffuseStr",  0.60f);
        shader.setFloat("material.specularStr", 0.55f);
        shader.setFloat("material.shininess",   48.0f);
        shader.setFloat("material.alpha",       1.0f);
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
        meshNames.push_back(mesh->mName.C_Str());
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    (void)scene;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex {};
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0,1,0);
        vertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0,0);
        vertex.Tangent = glm::vec3(1,0,0);
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
    return Mesh(std::move(vertices), std::move(indices), mesh->mName.C_Str());
}
