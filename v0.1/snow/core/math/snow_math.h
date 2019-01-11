#pragma once
#include <math.h>
#include <ctype.h>
// eigen
#ifdef _WIN32
#include <Eigen/Core>
#include <Eigen/Dense>
#else
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#endif
// self defined
#include "extypes.h"
#include "snow_glm.h"

namespace snow {

inline glm::vec3 AnyPerpendicularTo(glm::vec3 v) {
    // special case : (0, 0, 0)
    if (glm::all(glm::equal(v, glm::vec3(0, 0, 0)))) return glm::vec3(0, 1, 0);
    if (v.x != 0) { return glm::vec3(v.y / v.x, 1, 0); }
    if (v.y != 0) { return glm::vec3(1, v.x / v.y, 0); }
    if (v.z != 0) { return glm::vec3(1, 0, v.x / v.z); }
    return glm::vec3(0, 1, 0);
}

inline glm::quat QuatBetween(const glm::vec3 &vec0, const glm::vec3 &vec1, float angleScale=1.0) {
    auto _v0 = glm::normalize(vec0);
    auto _v1 = glm::normalize(vec1);
    float angle = std::acos(glm::dot(_v0, _v1));
    glm::vec3 axis = glm::normalize(glm::cross(_v0, _v1));
    if (std::isnan(angle))  angle = 0.f;
    if (std::isnan(axis.x)) axis = snow::AnyPerpendicularTo(_v0);
    auto q = glm::angleAxis(angle * angleScale, axis);
    return q;
}

inline size_t CeilLog2(size_t x) {
    static const size_t t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
    };
    size_t y = (((x & (x - 1)) == 0) ? 0 : 1);
    size_t j = 32;
    for (size_t i = 0; i < 6; i++) {
        size_t k = (((x & t[i]) == 0) ? 0 : j);
        y += k;  x >>= k;  j >>= 1;
    }
    return y;
}


}
