#pragma once

#include "snow_math.h"
#include <vector>
    
/* glm related stream functions */
inline std::ostream &operator<<(std::ostream &out, const glm::vec4 & vec)  { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::vec3 & vec)  { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::vec2 & vec)  { out << "[" << vec.x << ", " << vec.y << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::mat4 &mat)   { for (int r = 0; r < 4; ++r) { for (int c = 0; c < 4; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==3)? "]\n": ", "); }} return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::mat3 &mat)   { for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==2)? "]\n": ", "); }} return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::dvec4 & vec)  { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::dvec3 & vec) { out << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::dvec2 & vec) { out << "[" << vec.x << ", " << vec.y << "]"; return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::dmat4 &mat)  { for (int r = 0; r < 4; ++r) { for (int c = 0; c < 4; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==3)? "]\n": ", "); }} return out; }
inline std::ostream &operator<<(std::ostream &out, const glm::dmat3 &mat)  { for (int r = 0; r < 3; ++r) { for (int c = 0; c < 3; ++c) { out << ((c==0)?"[":"") << mat[c][r] << ((c==2)? "]\n": ", "); }} return out; }

/* std related stream functions */
template <typename T> inline std::ostream &operator<<(std::ostream &out, const std::vector<T> &vec) { for (size_t i = 0; i < vec.size(); ++i) out << ((i == 0)?"vector [" : ", ") << vec[i] << ((i==vec.size()-1)?"]" : ""); return out; }

/* snow related stream functions */
template <typename T> inline std::ostream& operator<<(std::ostream &out, const snow::_int3<T> &a)   { out << "[ " << a.x << " " << a.y << " " << a.z << " ]"; return out; }
template <typename T> inline std::ostream& operator<<(std::ostream &out, const snow::_int2<T> &a)   { out << "[ " << a.x << " " << a.y << " ]"; return out; }
template <typename T> inline std::ostream& operator<<(std::ostream &out, const snow::_float3<T> &a) { out << "[ " << a.x << " " << a.y << " " << a.z << " ]"; return out; }
template <typename T> inline std::ostream& operator<<(std::ostream &out, const snow::_float2<T> &a) { out << "[ " << a.x << " " << a.y << " ]"; return out; }

