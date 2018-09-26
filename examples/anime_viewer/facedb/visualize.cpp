#include "visualize.h"

void ShowModel::load() {
    mBilinearModel.updateMesh();

    std::vector<uint32_t> indices;
    std::vector<snow::Vertex> vertices;
    std::vector<snow::Texture> textures;

    for (int i = 0; i < FaceDB::NumVertices(); ++i)
        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, {0, 0} });

    for (int i = 0; i < FaceDB::Triangles().size(); ++i) {
        indices.push_back(FaceDB::Triangles()[i].x);
        indices.push_back(FaceDB::Triangles()[i].y);
        indices.push_back(FaceDB::Triangles()[i].z);
    }

    meshes.push_back(snow::Mesh::CreateMesh(vertices, indices, textures, true));

    updateFromTensor(mBilinearModel.mesh());
}

void ShowModel::updateFromTensor(const Tensor3 &tensor) {
    FaceDB::UpdateNormals(tensor);
    for (size_t i = 0; i < tensor.shape(0); i += 3) {
        meshes[0]->vertices[i / 3].position = { *tensor.data(i), *tensor.data(i+1), *tensor.data(i+2) };
    }
    for (int i = 0; i < FaceDB::NumVertices(); ++i) {
        auto norm = FaceDB::VertNormals()[i];
        meshes[0]->vertices[i].normal = { norm.x, norm.y, norm.z };
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
