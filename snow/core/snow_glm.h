#pragma once
// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "extypes.h"

namespace snow {

template <typename T> inline glm::vec<2, T> toGLM(const _float2<T> &v) { return {v.x, v.y       }; }
template <typename T> inline glm::vec<3, T> toGLM(const _float3<T> &v) { return {v.x, v.y, v.z  }; }

}