#pragma once
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <iomanip>
// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
// eigen
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
// self defined
#include "extypes.h"

namespace snow {

inline glm::dvec3 homoCoord(glm::dvec4 &q) { return glm::dvec3(q.x / q.w, q.y / q.w, q.z / q.w); }

inline glm::vec3 anyPerpendicularTo(glm::vec3 v) {
    // special case : (0, 0, 0)
    if (glm::all(glm::equal(v, glm::vec3(0, 0, 0)))) return glm::vec3(0, 1, 0);
    if (v.x != 0) { return glm::vec3(v.y / v.x, 1, 0); }
    if (v.y != 0) { return glm::vec3(1, v.x / v.y, 0); }
    if (v.z != 0) { return glm::vec3(1, 0, v.x / v.z); }
    return glm::vec3(0, 1, 0);
}

inline glm::quat quatBetween(const glm::vec3 &vec0, const glm::vec3 &vec1, float angleScale=1.0) {
    auto _v0 = glm::normalize(vec0);
    auto _v1 = glm::normalize(vec1);
    float angle = std::acos(glm::dot(_v0, _v1));
    glm::vec3 axis = glm::normalize(glm::cross(_v0, _v1));
    if (std::isnan(angle))  angle = 0.f;
    if (std::isnan(axis.x)) axis = snow::anyPerpendicularTo(_v0);
    auto q = glm::angleAxis(angle * angleScale, axis);
    return q;
}

}
