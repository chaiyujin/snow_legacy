#include "show.h"

void ShowWindow::setColor(const Image *imgPtr)                  { mColorPtr = imgPtr; }
void ShowWindow::setDepth(const Image *depthPtr)                { mDepthPtr = depthPtr; Image::ColorizeDepth(*mDepthPtr, mColorizedDepth); }
void ShowWindow::setPointCloud(const PointCloud *pointCloudPtr) { mPointCloudPtr = pointCloudPtr; updatePointCloud(); }
void ShowWindow::updateImageWithColor()                         { mImageShader.uploadImage(mColorPtr->data(), mColorPtr->width(), mColorPtr->height(), (mColorPtr->bpp() == 4) ? GL_RGBA : GL_RGB); }
void ShowWindow::updateImageWithDepth()                         { mImageShader.uploadImage(mColorizedDepth.data(), mColorizedDepth.width(), mColorizedDepth.height(), (mColorizedDepth.bpp() == 4) ? GL_RGBA : GL_RGB); }
void ShowWindow::updatePointCloud()                             { mPointShader.updateWithPointCloud(*mPointCloudPtr); }

void ShowWindow::processEvent(SDL_Event &event) {
}

void ShowWindow::draw() {
    ImGui::Begin("tools");
    glClear(GL_DEPTH_BUFFER_BIT);
    /* draw image */ if (mColorPtr) {
        bool show = false;
        ImGui::Checkbox("color", &mShowColor); 
        if (mDepthPtr) { ImGui::SameLine(); ImGui::Checkbox("depth", &mShowDepth); }
        if (mShowColor && mColorPtr != nullptr) { updateImageWithColor(); show = true; }
        if (mShowDepth && mDepthPtr != nullptr) { updateImageWithDepth(); show = true; }
        if (show) { mImageShader.use(); mImageShader.draw(); }
    }

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    /* draw point cloud */ if (mPointCloudPtr) {
        ImGui::Checkbox("point cloud", &mShowPoint);
        if (mShowPoint) {
            mPointShader.use();
            mPointShader.setMat4("Model", mModelMat);
            mPointShader.setMat4("View",  mViewMat);
            mPointShader.setMat4("Proj",  mProjMat);
            mPointShader.draw();
        }
    }

    /* draw model */ {
        glm::mat4 V(1.0); V[1][1] = V[2][2] = -1.0f;
        glm::mat4 normal = glm::transpose(glm::inverse(mModelMat * V));
        mModelShader.use();
        mModelShader.setMat4("Normal", normal);
        mModelShader.setMat4("Model", mModelMat * V);
        mModelShader.setMat4("View",  mViewMat);
        mModelShader.setMat4("Proj",  mProjMat);
        mModelShader.setVec3("LightPos", glm::vec3(0.0f, 0.0f, -30.0f));
        mModelShader.setVec3("Ambient",  glm::vec3(0.1f, 0.1f, 0.1f));
        mModelShader.setVec3("Diffuse",  glm::vec3(0.7f, 0.7f, 0.7f));
        mModelShader.draw();
    }
    ImGui::End();
}
