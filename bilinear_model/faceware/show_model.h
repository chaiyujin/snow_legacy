#pragma once
#include <snow.h>
#include <string>
#include <vector>
#include "tensor.h"
#include "faceware.h"

typedef std::vector<glm::vec3> Vertices;

class ShowModel : public snow::Model {
public:
    ShowModel() {}
    ~ShowModel() {}
    void load();
};

class ShowWindow : public snow::CameraWindow {
private:
    ShowModel        mModel;
    snow::Shader    *mShaderPtr;
public:
    ShowWindow();

    void draw() {
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
            glm::mat4 model = mModel.autoModelTransform(projection * view, 0.3);
            glm::mat4 normal = glm::transpose(glm::inverse(model));
            mShaderPtr->setMat4("model", model);
            mShaderPtr->setMat4("normal", normal);
            // draw model
            mModel.draw(*mShaderPtr);
        }
    }
};