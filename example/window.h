#pragma once
#include <snow.h>

using namespace snow;

class ObjWindow : public snow::AbstractWindow {
private:
    snow::Shader        *shader;
    snow::Model         *model;
    snow::ArcballCamera *cameraZPos, *cameraZNeg, *camera;
    int                  mCameraMode;
    std::string          mFilename;
    // gui
    float MoveSpeed, RotateSpeed, ZoomSpeed;
public:
    ObjWindow(const char *title="")
        : AbstractWindow(title), shader(nullptr), model(nullptr)
        , cameraZPos(nullptr),  cameraZNeg(nullptr), camera(nullptr), mCameraMode(1)
        , MoveSpeed(5.f), RotateSpeed(1.f), ZoomSpeed(1.f)
    {
        glEnable(GL_DEPTH_TEST);
        this->loadObj("../assets/nanosuit/nanosuit.obj");
    }

    void releaseObj() {
        if (model) {
            delete model;
            delete shader;
            delete cameraZPos;
            delete cameraZNeg;
            camera = nullptr;
        }
    }

    void loadObj(const std::string &fileName) {
        if (!std::ifstream(fileName).good()) {
            std::cerr << "No such file: " << fileName << std::endl;
            return;
        }
        
        this->glMakeCurrent();  // important !!

        this->releaseObj();
        this->model = new snow::Model(fileName);
        std::string vertGLSL = "../glsl/vert.glsl";
        std::string fragGLSL = "../glsl/frag.glsl";
        if (this->model->textures_loaded.size() == 0) {
            std::cout << "no tex\n";
            fragGLSL = "../glsl/frag_notex.glsl";
        }
        this->shader = new snow::Shader(vertGLSL, fragGLSL);
        // cameras
        this->cameraZPos = new snow::ArcballCamera(glm::vec3(0.f, 0.f,  3.f), glm::vec3(0.f,  1.f, 0.f));
        this->cameraZNeg = new snow::ArcballCamera(glm::vec3(0.f, 0.f, -3.f), glm::vec3(0.f, -1.f, 0.f));
        this->camera = this->cameraZNeg;
        mCameraMode = 0;
        mFilename = fileName;
        std::cout << "Open: " << fileName << std::endl;
    }

    void processEvent(SDL_Event &event) {
        if (event.type == SDL_DROPFILE) {
            std::string file = event.drop.file;
            std::cout << event.drop.windowID << " " << SDL_GetWindowID(this->windowPtr()) << std::endl;
            loadObj(file);
        }
        else if (camera)
            camera->processMouseEvent(event);
    }

    void draw() {
        glEnable(GL_DEPTH_TEST);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    auto files = snow::FileDialog({"obj"}, false, false);
                    if (files.size() > 0)
                        loadObj(files[0]);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (model) {
            if (mCameraMode == 0) 
                this->camera = this->cameraZPos;
            else
                this->camera = this->cameraZNeg;
            this->shader->use();
            this->shader->setVec3("lightPos", this->camera->eye());
            this->shader->setVec3("lightColor", glm::vec3(1.f, 1.f, 1.f));
            // view/projection transformations
            int w, h;
            SDL_GetWindowSize(mWindowPtr, &w, &h);
            glm::mat4 projection = this->perspective(camera);
            glm::mat4 view = camera->viewMatrix();
            this->shader->setMat4("projection", projection);
            this->shader->setMat4("view", view);

            // render the loaded model
            glm::mat4 model = this->model->autoModelTransform(projection * view);
            glm::mat4 normal = glm::transpose(glm::inverse(model));
            this->shader->setMat4("model", model);
            this->shader->setMat4("normal", normal);
            this->model->draw(*shader);

            {
                ImGui::Begin("utils");
                ImGui::TextWrapped("%s", (std::string("Path:") + mFilename).c_str());
                ImGui::BeginGroup();
                if (ImGui::RadioButton("Camera (at +z)", mCameraMode == 0)) { mCameraMode = 0; } ImGui::SameLine();
                if (ImGui::RadioButton("Camera (at -z)", mCameraMode == 1)) { mCameraMode = 1; }
                
                ImGui::DragFloat("Speed Zoom",   &ZoomSpeed,   0.1f, 0.5f, 5.0f);
                ImGui::DragFloat("Speed Move",   &MoveSpeed,   0.1f,  1.f, 10.f);
                ImGui::DragFloat("Speed Rotate", &RotateSpeed, 0.1f, 0.5f, 5.0f);
                this->camera->setSpeedMove(MoveSpeed);
                this->camera->setSpeedRotate(RotateSpeed);
                this->camera->setSpeedZoom(ZoomSpeed);
                ImGui::EndGroup();
                ImGui::End();
            }
        }
        else {
            ImGui::Text("Drag or open a .obj file.");
        }
    }
};
