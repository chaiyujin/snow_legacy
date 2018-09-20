#include "show_model.h"
#include "glsl.h"

void ShowModel::load() {
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

ShowWindow::ShowWindow()
    : snow::CameraWindow("")
    , mGLModel()
    , mBilinearModel(1)
    , mIden(FaceDB::LengthIdentity)
    , mExpr(FaceDB::LengthExpression)
{
    DrawArcball = false;
    mShaderPtr = new snow::Shader();
    mShaderPtr->buildFromCode(VERT_GLSL, FRAG_NOTEX_GLSL);

    updateShowParameters(
        mBilinearModel.idenParameter().param(),
        mBilinearModel.exprParameter(0).param()
    );
}

void ShowWindow::draw() {
    {
        glEnable(GL_DEPTH_TEST);
        mShaderPtr->use();
        mShaderPtr->setVec3("lightPos", mCamera.eye());
        mShaderPtr->setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.9f));
        // view. projection
        glm::mat4 projection = this->perspective(&mCamera);
        glm::mat4 view = mCamera.viewMatrix();
        mShaderPtr->setMat4("projection", projection);
        mShaderPtr->setMat4("view", view);
        // model, normal
        glm::mat4 model = mGLModel.autoModelTransform(projection * view, 0.3);
        glm::mat4 normal = glm::transpose(glm::inverse(model));
        mShaderPtr->setMat4("model", model);
        mShaderPtr->setMat4("normal", normal);
        // draw model
        mGLModel.draw(*mShaderPtr);
    }
    {
        bool changed = false;
        ImGui::SetNextWindowPos (ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(130, ImGui::GetIO().DisplaySize.y / 2 - 1));
        ImGui::Begin("identity", nullptr, ImGuiWindowFlags_NoResize);
        for (size_t i = 1; i < mIden.size(); ++i) {
            ImGui::PushItemWidth(80);
            if (ImGui::SliderFloat((std::to_string(i)).c_str(), &mIden[i], -3, 3))
                changed = true;
        }
        ImGui::End();
        ImGui::SetNextWindowPos (ImVec2(130, 0));
        ImGui::SetNextWindowSize(ImVec2(130, ImGui::GetIO().DisplaySize.y / 2 - 1));
#ifdef PARAMETER_FACS
        const char *exprName = "FACS";
        const float vmin = 0.0, vmax = 1.0;
#else
        const char * = "expression";
        const float vmin = -3.0, vmax = 3.0;
#endif
        ImGui::Begin(exprName, nullptr, ImGuiWindowFlags_NoResize);
        for (size_t i = 1; i < mExpr.size(); ++i) {
            ImGui::PushItemWidth(80);
            if (ImGui::SliderFloat((std::to_string(i)).c_str(), &mExpr[i], vmin, vmax))
                changed = true;
        }
        ImGui::End();
        if (changed)
            updateShowParameters(mIden.data(), mExpr.data());
    }
}