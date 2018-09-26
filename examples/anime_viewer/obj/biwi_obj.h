#pragma once
#include <snow.h>
#include <string>
#include <vector>

typedef std::vector<glm::vec3> Vertices;

Vertices read_vl(std::string filepath);

class ObjMesh : public snow::Model {
    void modifyPosition(const std::vector<glm::vec3> &new_positions);
public:
    ObjMesh(std::string filename);
    ~ObjMesh() {
        for (size_t i = 0; i < textures_loaded.size(); ++i) {
            GLuint tid = textures_loaded[i].id;
            glDeleteTextures(1, &tid);
        }
    }

    void modify(const void *userData) {
        modifyPosition(*(const std::vector<glm::vec3> *)userData);
    }
};
