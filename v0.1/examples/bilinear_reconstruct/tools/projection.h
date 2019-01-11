#pragma once
#include <snow.h>

template <typename T>
inline void projectToImageSpace(const T *vertices, int numVertices,
                                const glm::dmat4 &pvm,
                                std::vector<snow::float2> &points) {                            
    // projection
    points.resize(numVertices);
    for (size_t iVert = 0; iVert < numVertices; ++iVert) {
        glm::vec4 p = { (float)vertices[0], (float)vertices[1], (float)vertices[2], 1.0f };
        p = pvm * p;
        p.x /= p.w; p.y /= p.w; p.z /= p.w;
        points[iVert].x = p.x;
        points[iVert].y = p.y;
        vertices += 3;
    }
}

template <typename T>
inline void projectToImageSpace(const T *vertices, int numVertices,
                                const glm::dmat4 &projection,
                                const glm::dmat4 &view,
                                const glm::dmat4 &model,
                                std::vector<snow::float2> &points) {                            
    // projection
    const glm::mat4 mat = projection * view * model;
    projectToImageSpace(vertices, numVertices, mat, points);
}

template <typename T>
inline void projectToImageSpace(const std::vector<snow::_float3<T>> &vertices,
                                const glm::dmat4 &pvm,
                                std::vector<snow::float2> &points) {
    projectToImageSpace(&vertices[0], (int)vertices.size(), pvm, points);
}

template <typename T>
inline void projectToImageSpace(const std::vector<snow::_float3<T>> &vertices,
                                const glm::dmat4 &projection,
                                const glm::dmat4 &view,
                                const glm::dmat4 &model,
                                std::vector<snow::float2> &points) {
    projectToImageSpace(&vertices[0], (int)vertices.size(), projection, view, model, points);
}