#include "show_model.h"
#include "glsl.h"

void ShowModel::load() {

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

        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv0, {0, 0, 0} });
        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv1, {0, 0, 0} });
        vertices.push_back({ {0, 0, 0}, {0, 0, 0}, uv2, {0, 0, 0} });
        indices.push_back(i * 3);
        indices.push_back(i * 3 + 1);
        indices.push_back(i * 3 + 2);
    }

    // texture
    {
        std::string filename = snow::path::Join(FaceDB::RootDir(), "diffuse.bmp");
        if (std::ifstream(filename).good()) {
            // texture
            snow::Texture texture;
            texture.id = snow::TextureFromFile(filename.c_str(), "", true);
            texture.type = "texture_diffuse";
            texture.path = filename;
            textures_loaded.push_back(texture);
            textures.push_back(texture);
        }
        
        filename = snow::path::Join(FaceDB::RootDir(), "normal.bmp");
        if (std::ifstream(filename).good()) {
            // texture
            snow::Texture texture;
            texture.id = snow::TextureFromFile(filename.c_str(), "", true);
            texture.type = "texture_normal";
            texture.path = filename;
            textures_loaded.push_back(texture);
            textures.push_back(texture);
        }
    }

    meshes.push_back(snow::Mesh::CreateMesh(vertices, indices, textures, true));
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
        const auto v = FaceDB::Triangles()[i];
        const glm::vec3 p[3] = { getVert(v[0]), getVert(v[1]), getVert(v[2]) };
        const glm::vec3 n[3] = { getNorm(v[0]), getNorm(v[1]), getNorm(v[2]) };
        int vt0 = FaceDB::TrianglesUV()[i].x;
        int vt1 = FaceDB::TrianglesUV()[i].y;
        int vt2 = FaceDB::TrianglesUV()[i].z;
        glm::vec2 uv0 = snow::toGLM(FaceDB::TexCoords()[vt0]);
        glm::vec2 uv1 = snow::toGLM(FaceDB::TexCoords()[vt1]);
        glm::vec2 uv2 = snow::toGLM(FaceDB::TexCoords()[vt2]);

        // calc tangents
        glm::vec3 edge1 = p[1] - p[0];
        glm::vec3 edge2 = p[2] - p[0];
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent1;
        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        // set vertices
        for (int j = 0; j < 3; ++j) {
            meshes[0]->vertices[i*3+j].position = p[j];
            meshes[0]->vertices[i*3+j].normal   = n[j];
            meshes[0]->vertices[i*3+j].tangent  = tangent1;
        }
    }
    glBindVertexArray(meshes[0]->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, meshes[0]->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, meshes[0]->vertices.size() * sizeof(snow::Vertex), &meshes[0]->vertices[0]);
}

VisualizerWindow::VisualizerWindow()
    : snow::CameraWindow("")
    , mGLModel()
    , mBilinearModel()
    , mIden(FaceDB::LengthIdentity)
    , mExpr(FaceDB::LengthExpression)
{
    mBilinearModel.appendModel(1);
    mBilinearModel.prepareAllModel();
    
    DrawArcball = false;
    mShaderPtr = new snow::Shader();
    mShaderPtr->buildFromCode(VERT_GLSL, FRAG_NOTEX_GLSL);

    updateShowParameters(
        mBilinearModel.idenParameter().param(),
        mBilinearModel.exprParameter(0).param()
    );
}

void VisualizerWindow::draw() {
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
        glm::mat4 model = mGLModel.autoModelTransform(projection * view, 0.3f);
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
        const char *exprName = "expression";
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