#pragma once
#include <string>
#include <vector>
// snow
#include "../../core/snow_core.h"
#include "../tools/snow_shader.h"

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

    uint32_t TextureFromFile(const char *path, const std::string &directory, bool gamma);
    
    class Mesh {
    public:
        /* gl object */
        uint32_t VAO;
        /* gl objects */
        uint32_t VBO, EBO;
        bool is_dynamic;
        /* mesh data */
        std::vector<Vertex> vertices;
        std::vector<Texture> textures;
        std::vector<uint32_t> indices;
        /* function */
        virtual void draw(Shader &shader);
        virtual ~Mesh();

        static Mesh *CreateMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                                const std::vector<Texture> &textures, bool dynamic=false) {
            return new Mesh(vertices, indices, textures, dynamic);
        }
    protected:
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<Texture> &textures, bool dynamic=false);
        /* function */
        void setupMesh();
    };
}