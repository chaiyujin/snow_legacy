#pragma once
#include <snow.h>


class ObjMesh : public snow::Model {
    public:
        ObjMesh(std::string filename);
        void modifyPosition(const std::vector<glm::vec3> &new_positions);
};
