// snow
#include "snow_model.h"

namespace snow {
    void Model::loadModel(const std::string &path) {
        Assimp::Importer importer;
        const aiScene * scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            throw std::runtime_error("ASSIMP error");
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void Model::processNode(aiNode *node, const aiScene *scene) {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            // push the processed mesh
            meshes.push_back(processMesh(mesh, scene));
        }
        // recursive
        for (uint32_t i = 0; i < node->mNumChildren; ++i)
            processNode(node->mChildren[i], scene);
    }

    Mesh *Model::processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture> textures;

        // walk through all vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex;
            {
                glm::vec3 vec;
                // position
                vec.x = mesh->mVertices[i].x;
                vec.y = mesh->mVertices[i].y;
                vec.z = mesh->mVertices[i].z;
                vertex.position = vec;
                // normal
                vec.x = mesh->mNormals[i].x;
                vec.y = mesh->mNormals[i].y;
                vec.z = mesh->mNormals[i].z;
                vertex.normal = vec;
            }
            if (mesh->mTextureCoords[0]) {
                // texture coord
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.tex_coords = vec;
            }
            vertices.push_back(vertex);
        }
        // all mesh's faces (triangles)
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }
        // process matericals
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            // 1. diffuse maps
            std::vector<Texture> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
            // 2. specular maps
            std::vector<Texture> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
            // 3. normal maps
            std::vector<Texture> normal_maps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
            // 4. height maps
            std::vector<Texture> height_maps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
            textures.insert(textures.end(), height_maps.begin(), height_maps.end());
        }
        return Mesh::CreateMesh(vertices, indices, textures);
    }

    std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string type_name) {
        std::vector<Texture> textures;
        for (uint32_t i = 0; mat && i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if the texture is loaded before
            bool skip = false;
            for (uint32_t j = 0; j < textures_loaded.size(); ++j) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory, this->gamma_correction);
                texture.type = type_name;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }

    glm::vec3 Model::calcMeanCenter() {
        double cnt = 0;
        glm::dvec3 sum(0, 0, 0);
        for (auto &mesh: meshes) {
            for (auto &vert : mesh->vertices) {
                sum += vert.position;
                cnt += 1;
            }
        }
        center = glm::vec3(sum.x / cnt, sum.y / cnt, sum.z / cnt);
        return center;
    }

    glm::vec2 Model::_calcFarestPosition(const glm::mat4 &projView) {
        auto project = [&](const glm::vec3 p) -> glm::vec2 {
            auto Q = projView * glm::vec4(p.x, p.y, p.z, 1.0);
            return glm::vec2(Q.x/Q.z, Q.y/Q.z);
        };
        float cnt = 0;
        farest = glm::vec2(0, 0);
        for (auto &mesh: meshes) {
            for (auto &vert : mesh->vertices) {
                glm::vec2 delta = project(vert.position - center);
                delta.x = std::abs(delta.x);
                delta.y = std::abs(delta.y);
                farest += delta;
                cnt += 1;
            }
        }
        farest = glm::vec2(farest.x / cnt, farest.y / cnt);
        return farest;
    }

    glm::mat4 Model::autoModelTransform(const glm::mat4 &projView) {
        if (initTransform[0][0] > 0)
            return initTransform;        
        calcMeanCenter();
        farest = _calcFarestPosition(projView);
        float x = 0.5 / farest.x;
        float y = 0.5 / farest.y;
        float scale = std::min(x, y);
        glm::mat4 trans(1.0);
        // std::cout << scale << " "
        //           << center.x << " "
        //           << center.y << " "
        //           << center.z << " " << std::endl;
        trans = glm::scale(trans, glm::vec3(scale, scale, scale));	// it's a bit too big for our scene, so scale it down
        trans = glm::translate(trans, -center); // translate it down so it's at the center of the scene
        initTransform = trans;
        return initTransform;
    }
}