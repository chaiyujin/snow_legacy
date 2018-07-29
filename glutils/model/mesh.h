#pragma once
#include <string>
#include <vector>
#include <core/math.h>
#include <shader/shader.h>

namespace snow {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coords;
    };

    struct Texture {
        uint32_t id;
        std::string type;
        std::string path;
    };

    class Mesh {
    public:
        /* gl object */
        uint32_t VAO;
        bool is_dynamic;
        /* mesh data */
        std::vector<Vertex> vertices;
        std::vector<Texture> textures;
        std::vector<uint32_t> indices;
        /* function */
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<Texture> &textures, bool dynamic=false);
        void draw(Shader &shader);
    private:
        /* gl objects */
        uint32_t VBO, EBO;
        /* function */
        void setupMesh();
    };
}