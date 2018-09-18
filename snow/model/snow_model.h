#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
// thidr-party
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// snow
#include "snow_mesh.h"
#include "../core/snow_math.h"
#include "../gui/snow_shader.h"

namespace snow {
    
    class Model
    {
    public:
        std::vector<Texture> textures_loaded;
        std::vector<Mesh * > meshes;
        std::string directory;
        bool gamma_correction;
        glm::vec3 center;
        glm::mat4 initTransform;

        Model() : gamma_correction(false), initTransform(0.0) {}
        Model(const std::string &path, bool gamma=false) : gamma_correction(gamma) {
            this->loadModel(path);
            initTransform = glm::mat4(0.0);
            farest = glm::vec2(0, 0);
            calcMeanCenter();
        }
        virtual ~Model() {
            for (size_t i = 0; i < meshes.size(); ++i)
                delete meshes[i];
            meshes.clear();
            for (size_t i = 0; i < textures_loaded.size(); ++i) {
                GLuint tid = textures_loaded[i].id;
                glDeleteTextures(1, &tid);
            }
            textures_loaded.clear();
        }
        void draw(Shader &shader) {
            for (size_t i = 0; i < meshes.size(); ++i) {
                meshes[i]->draw(shader);
            }
        }
        glm::vec3 calcMeanCenter();
        // auto translation and scale for visualization
        glm::mat4 autoModelTransform(const glm::mat4 &projView, float ratio=0.5f);

    protected:
        glm::vec2 farest;
        glm::vec2 _calcFarestPosition(const glm::mat4 &projView);

        void loadModel(const std::string &path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh *processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string type_name);
    };
}