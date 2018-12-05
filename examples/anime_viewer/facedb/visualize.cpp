#include "visualize.h"
#include <fstream>

void ShowModel::load() {
    mBilinearModel.updateMesh();

    std::vector<uint32_t> indices;
    std::vector<snow::Vertex> vertices;
    std::vector<snow::Texture> textures;

    for (int i = 0; i < FaceDB::Triangles().size(); ++i) {
        int vt0 = FaceDB::TrianglesUV()[i].x;
        int vt1 = FaceDB::TrianglesUV()[i].y;
        int vt2 = FaceDB::TrianglesUV()[i].z;
        glm::vec2 uv0 = snow::toGLM(FaceDB::TexCoords()[vt0]);
        glm::vec2 uv1 = snow::toGLM(FaceDB::TexCoords()[vt1]);
        glm::vec2 uv2 = snow::toGLM(FaceDB::TexCoords()[vt2]);

        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv0 });
        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv1 });
        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv2 });
        indices.push_back(i * 3);
        indices.push_back(i * 3 + 1);
        indices.push_back(i * 3 + 2);
    }

    // texture
    std::string filename = snow::path::Join(FaceDB::RootDir(), "diffuse.bmp");
    if (std::ifstream(filename).good()) {
        // texture
        snow::Texture texture;
        texture.id = snow::TextureFromFile(filename.c_str(), "", true);
        texture.type = "type_name";
        texture.path = filename;
        textures_loaded.push_back(texture);
        textures.push_back(texture);
    }
    meshes.push_back(snow::Mesh::CreateMesh(vertices, indices, textures, true));

    updateFromTensor(mBilinearModel.mesh());
}

void ShowModel::updateFromTensor(const Tensor3 &tensor) {
    FaceDB::UpdateNormals(tensor);
    auto getVert = [&](int vi) -> glm::vec3 {
        return {
             (float)*tensor.data(vi*3),
             (float)*tensor.data(vi*3+1),
             (float)*tensor.data(vi*3+2)
        };
    };
    auto getNorm = [&](int vi) -> glm::vec3 {
        auto norm = FaceDB::VertNormals()[vi];
        return { norm.x, norm.y, norm.z };
    };
    for (int i = 0; i < FaceDB::Triangles().size(); ++i) {
        int v0 = FaceDB::Triangles()[i].x;
        int v1 = FaceDB::Triangles()[i].y;
        int v2 = FaceDB::Triangles()[i].z;
        meshes[0]->vertices[i*3].position = getVert(v0);
        meshes[0]->vertices[i*3].normal = getNorm(v0);
        meshes[0]->vertices[i*3+1].position = getVert(v1);
        meshes[0]->vertices[i*3+1].normal = getNorm(v1);
        meshes[0]->vertices[i*3+2].position = getVert(v2);
        meshes[0]->vertices[i*3+2].normal = getNorm(v2);
    }
    glBindVertexArray(meshes[0]->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, meshes[0]->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, meshes[0]->vertices.size() * sizeof(snow::Vertex), &meshes[0]->vertices[0]);
}

void ShowModel::updateIden(const std::vector<double> &iden) {
    if (iden.size() != FaceDB::LengthIdentity)
        throw std::runtime_error("<iden> size() is not correct!");
    mBilinearModel.updateIdenOnCore(0, iden.data());
    mBilinearModel.updateExpr(0);
    mBilinearModel.updateScale(0);
    mBilinearModel.rotateYXZ(0);
    mBilinearModel.translate(0);
    updateFromTensor(mBilinearModel.mesh());
}

void ShowModel::updateExpr(const std::vector<double> &expr) {
    if (expr.size() != FaceDB::LengthExpression)
        throw std::runtime_error("<expr> size() is not correct!");
    // std::cout << expr << std::endl;
    mBilinearModel.updateExpr(0, expr.data());
    mBilinearModel.updateScale(0);
    mBilinearModel.rotateYXZ(0);
    mBilinearModel.translate(0);
    updateFromTensor(mBilinearModel.mesh());
}
