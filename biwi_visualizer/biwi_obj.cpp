#include "biwi_obj.h"
#include <fstream>
using namespace std;
using namespace snow;


    ObjMesh::ObjMesh(std::string filename) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture> textures;
        ifstream fin(filename);
        if (fin.is_open()) {
            // read vertices
            int cnt_v = -1, cnt_vn = -1, cnt_vt = -1;
            while (!fin.eof()) {
                std::string type;
                std::string line;
                fin >> type;
                std::getline(fin, line);
                Trim(line);
                if (type == "v") {
                    glm::vec3 pos;
                    sscanf(line.c_str(), "%f %f %f", &pos.x, &pos.y, &pos.z);
                    cnt_v ++;
                    if (cnt_v >= vertices.size()) {
                        Vertex vert;
                        vertices.push_back(vert);
                    }
                    vertices[cnt_v].position = pos;
                }
                else if (type == "vn") {
                    glm::vec3 norm;
                    sscanf(line.c_str(), "%f %f %f", &norm.x, &norm.y, &norm.z);
                    cnt_vn ++;
                    if (cnt_vn >= vertices.size()) {
                        Vertex vert;
                        vertices.push_back(vert);
                    }   
                    vertices[cnt_vn].normal = norm;
                }
                else if (type == "vt") {
                    glm::vec2 tex;
                    sscanf(line.c_str(), "%f %f", &tex.x, &tex.y);
                    cnt_vt ++;
                    if (cnt_vt >= vertices.size()) {
                        Vertex vert;
                        vertices.push_back(vert);
                    }
                    vertices[cnt_vt].tex_coords = tex;
                }
                else if (type == "f") {
                    int x0, y0, z0;
                    int x1, y1, z1;
                    int x2, y2, z2;
                    sscanf(line.c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d",
                           &x0, &y0, &z0,
                           &x1, &y1, &x1,
                           &x2, &y2, &z2);
                    indices.push_back(x0-1);
                    indices.push_back(x1-1);
                    indices.push_back(x2-1);
                }
            }
            fin.close();
            filename = filename.substr(0, filename.length() - 3) + "png";
            if (ifstream(filename).good()) {
                // texture
                Texture texture;
                texture.id = TextureFromFile(filename.c_str(), "", true);
                texture.type = "type_name";
                texture.path = filename;
                textures_loaded.push_back(texture);
                textures.push_back(texture);
            }
            meshes.push_back(Mesh::CreateMesh(vertices, indices, textures, true));
        }
    }

    void ObjMesh::modifyPosition(const std::vector<glm::vec3> &new_positions) {
        for (size_t i = 0; i < new_positions.size(); ++i) {
            meshes[0]->vertices[i].position = new_positions[i];
        }
        glBindVertexArray(meshes[0]->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[0]->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, meshes[0]->vertices.size() * sizeof(Vertex), &meshes[0]->vertices[0]);
    }