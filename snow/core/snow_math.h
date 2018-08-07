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