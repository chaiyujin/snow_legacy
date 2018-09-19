#include "show_model.h"


std::string VERT_GLSL =
"    layout (location = 0) in vec3 aPos;"
"    layout (location = 1) in vec3 aNormal;"
"    layout (location = 2) in vec2 aTexCoords;"

"    out vec2 TexCoords;"
"    out vec3 FragPos;"
"    out vec3 Normal;"

"    uniform mat4 model;"
"    uniform mat4 view;"
"    uniform mat4 projection;"
"    uniform mat4 normal;"

"    void main()"
"    {"
"        TexCoords = aTexCoords;  "  
"        gl_Position = projection * view * model * vec4(aPos, 1.0);"
"        FragPos = vec3(model * vec4(aPos, 1.0));"
"        Normal = mat3(normal) * aNormal;"
"    }";

std::string FRAG_GLSL =
"    out vec4 FragColor;"

"    in vec2 TexCoords;"
"    in vec3 FragPos;"
"    in vec3 Normal;"

"    uniform vec3        lightPos;"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.1;"
"        vec3 ambient = ambientStrength * lightColor;"

"        vec3 norm = normalize(Normal);"
"        vec3 lightDir = normalize(lightPos - FragPos);"
"        float diff = max(dot(norm, lightDir), 0.0);"
"        vec3 diffuse = diff * lightColor;"

"        vec3 result = (ambient + diffuse);"
"        FragColor = vec4(result, 1.0) * texture(texture_diffuse1, TexCoords);"
"    }";

std::string FRAG_NOTEX_GLSL =
"    out vec4 FragColor;"

"    in vec2 TexCoords;"
"    in vec3 FragPos;"
"    in vec3 Normal;"

"    uniform vec3        lightPos;"
"    uniform vec3        lightColor;"
"    uniform sampler2D   texture_diffuse1;"

"    void main()"
"    {   "
"        float ambientStrength = 0.1;"
"        vec3 ambient = ambientStrength * lightColor;"

"        vec3 norm = normalize(Normal);"
"        vec3 lightDir = normalize(lightPos - FragPos);"
"        float diff = max(dot(norm, lightDir), 0.0);"
"        vec3 diffuse = diff * lightColor;"

"        vec3 result = (ambient + diffuse);"
"        FragColor = vec4(result, 1.0);"
"    }";



void  ShowModel::load() {
    std::vector<snow::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<snow::Texture> textures;

    const auto &core = FaceDB::core_tensor();
    double * iden = snow::alignedMalloc<double>(75);
    double * expr = snow::alignedMalloc<double>(45);
    memset(iden, 0, sizeof(double) * 75);
    memset(expr, 0, sizeof(double) * 45);
    iden[0] = 1;
    iden[1] = -1;
    expr[0] = 0.1;

    Tensor3 t1;
    
    core.mulVec(iden, expr, t1);
    
    FaceDB::update_gl_normal(t1, nullptr);
    for (int i = 0; i < t1.shape(0); i += 3) {
        auto norm = FaceDB::v_normals()[i / 3];
        vertices.push_back({
            {t1.data(i)[0], t1.data(i)[1], t1.data(i)[2]},
            {norm.x, norm.y, norm.z},
            {0, 0}
        });
    }

    for (int i = 0; i < FaceDB::triangles().size(); ++i) {
        indices.push_back(FaceDB::triangles()[i].x);
        indices.push_back(FaceDB::triangles()[i].y);
        indices.push_back(FaceDB::triangles()[i].z);
    }

    meshes.push_back(snow::Mesh::CreateMesh(vertices, indices, textures, true));
}

void ShowModel::updateFromTensor(const Tensor3 &tensor) {
    for (size_t i = 0; i < tensor.shape(0); i += 3) {
        meshes[0]->vertices[i / 3].position = { tensor.data(i)[0], tensor.data(i)[1], tensor.data(i)[2] };
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
        ImGui::Begin("expression", nullptr, ImGuiWindowFlags_NoResize);
        for (size_t i = 1; i < mExpr.size(); ++i) {
            ImGui::PushItemWidth(80);
            if (ImGui::SliderFloat((std::to_string(i)).c_str(), &mExpr[i], -3, 3))
                changed = true;
        }
        ImGui::End();
        if (changed)
            updateShowParameters(mIden.data(), mExpr.data());
    }
}