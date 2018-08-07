#pragma once

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <core/math.h>
#include <model/mesh.h>
#include <core/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


namespace snow {
    uint32_t TextureFromFile(const char *path, const std::string &directory, bool gamma);
    
    class Model
    {
    public:
        std::vector<Texture> textures_loaded;
        std::vector<Mesh> meshes;
        std::string directory;
        bool gamma_correction;

        Model(const std::string &path, bool gamma=false) : gamma_correction(gamma) {
            this->loadModel(path);
        }
        void draw(Shader &shader) {
            for (size_t i = 0; i < meshes.size(); ++i) {
                meshes[i].draw(shader);
            }
        }

    private:
        void loadModel(const std::string &path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string type_name);
    };
}