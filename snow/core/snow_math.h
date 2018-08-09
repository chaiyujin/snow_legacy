#pragma once
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <iomanip>
// third-party
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace snow {
    inline glm::vec3 anyPerpendicularTo(glm::vec3 v) {
        // special case : (0, 0, 0)
        if (glm::all(glm::equal(v, glm::vec3(0, 0, 0)))) return glm::vec3(0, 1, 0);
        if (v.x != 0) { return glm::vec3(v.y / v.x, 1, 0); }
        if (v.y != 0) { return glm::vec3(1, v.x / v.y, 0); }
        if (v.z != 0) { return glm::vec3(1, 0, v.x / v.z); }
    }

    inline glm::quat quatBetween(const glm::vec3 &_v0, const glm::vec3 &_v1) {
        auto v0 = glm::normalize(_v0);
        auto v1 = glm::normalize(_v1);
        float angle = std::acos(glm::dot(v0, v1));
        glm::vec3 axis = glm::normalize(glm::cross(v0, v1));
        if (std::isnan(angle))  angle = 0.f;
        if (std::isnan(axis.x)) axis = snow::anyPerpendicularTo(v0);
        auto q = glm::angleAxis(angle, axis);
        // std::cout << "angle: " << angle << std::endl;
        // std::cout << "axis:  " << axis.x << " " << axis.y << " " << axis.z << std::endl;
        // std::cout << "quat:  " << q.x << " " << q.y << " " << q.z << " " << q.w << std::endl;
        return q;
    }

    /* glm related stream functions */
    inline std::ostream &operator<<(std::ostream &out, const glm::vec3 & vec) { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]"; return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::vec2 & vec) { out << "[" << vec.x << ", " << vec.y << "]"; return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::mat4 &mat) { for (int r = 0; r < 4; ++r) { for (int c = 0; c < 4; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==3)? "]\n": ", "); }} return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::mat3 &mat) { for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==2)? "]\n": ", "); }} return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::dvec3 & vec) { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]"; return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::dvec2 & vec) { out << "[" << vec.x << ", " << vec.y << "]"; return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::dmat4 &mat) { for (int r = 0; r < 4; ++r) { for (int c = 0; c < 4; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==3)? "]\n": ", "); }} return out; }
    inline std::ostream &operator<<(std::ostream &out, const glm::dmat3 &mat) { for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==2)? "]\n": ", "); }} return out; }
}