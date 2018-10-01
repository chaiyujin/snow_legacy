#pragma once
#include <snow.h>

template <typename T>
inline void projectToImageSpace(const T *vertices, int numVertices,
                                const glm::mat4 projection,
                                const glm::mat4 view,
                                const glm::mat4 model,
                                std::vector<snow::float2> &points) {                            
    // projection
    const glm::mat4 mat = projection * view * model;
    points.resize(numVertices);
    for (size_t iVert = 0; iVert < numVertices; ++iVert) {
        glm::vec4 p = { (float)vertices[0], (float)vertices[1], (float)vertices[2], 1.0f };
        p = mat * p;
        p.x /= p.w; p.y /= p.w; p.z /= p.w;
        points[iVert].x = p.x;
        points[iVert].y = p.y;
        vertices += 3;
    }
}

template <typename T>
inline void projectToImageSpace(const std::vector<snow::_float3<T>> &vertices,
                                const glm::mat4 projection,
                                const glm::mat4 view,
                                const glm::mat4 model,
                                std::vector<snow::float2> &points) {
    projectToImageSpace(&vertices[0], vertices.size(), projection, view, model, points);
}