// third-party
#include <glad/glad.h>
#include <stb_image.h>
// snow
#include "snow_mesh.h"

namespace snow {
    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<Texture> &textures, bool dynamic) {
        this->vertices = vertices;
        this->textures = textures;
        this->indices = indices;
        this->is_dynamic = dynamic;
        this->setupMesh();
    }

    Mesh::~Mesh() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Mesh::setupMesh() {
        GLenum usage = (this->is_dynamic)? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        // gen buffer 
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], usage);

        // vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // tex_coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

        // unbind VAO
        glBindVertexArray(0);
    }

    void Mesh::draw(Shader &shader) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // bind texture id
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);

            shader.setInt((name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        glActiveTexture(GL_TEXTURE0);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    uint32_t TextureFromFile(const char *path, const std::string &directory, bool gamma) {
        std::string filename(path);
        if (directory.length() > 0)
            filename = directory + '/' + filename;

        uint32_t texture_id;
        int width, height, nr_components;
        glGenTextures(1, &texture_id);
        uint8_t *data = stbi_load(filename.c_str(), &width, &height, &nr_components, 0);
        if (data) {
            GLenum format;
            switch (nr_components) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default:
                std::cerr << "[Texture]: unsupported nr_components " << nr_components << std::endl;
                throw std::runtime_error("texture error");
            }

            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else {
            std::cerr << "[Texture]: failed to load at path " << path << std::endl;
            stbi_image_free(data);
            throw std::runtime_error("texture error");
        }
        return texture_id;
    }
}