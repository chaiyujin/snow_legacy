#include "show.h"

void ShowWindow::setColor(const Image *imgPtr)      { mColorPtr = imgPtr; }
void ShowWindow::setDepth(const Image *depthPtr)    { mDepthPtr = depthPtr; Image::ColorizeDepth(*mDepthPtr, mColorizedDepth); }
void ShowWindow::setPointCloud(const PointCloud *pointCloudPtr) { mPointCloudPtr = pointCloudPtr; updatePointCloud(); }
void ShowWindow::updateImageWithColor() { mImageShader.uploadImage(mColorPtr->data(), mColorPtr->width(), mColorPtr->height(), (mColorPtr->bpp() == 4) ? GL_RGBA : GL_RGB); }
void ShowWindow::updateImageWithDepth() { mImageShader.uploadImage(mColorizedDepth.data(), mColorizedDepth.width(), mColorizedDepth.height(), (mColorizedDepth.bpp() == 4) ? GL_RGBA : GL_RGB); }
void ShowWindow::updatePointCloud()     { mPointShader.updateWithPointCloud(*mPointCloudPtr); }

void ShowWindow::draw() {
    /* draw image */ if (mColorPtr) {
        bool show = false;
        ImGui::Checkbox("color", &mShowColor); 
        if (mDepthPtr) { ImGui::SameLine(); ImGui::Checkbox("depth", &mShowDepth); }
        if (mShowColor && mColorPtr != nullptr) { updateImageWithColor(); show = true; }
        if (mShowDepth && mDepthPtr != nullptr) { updateImageWithDepth(); show = true; }
        if (show) {
            mImageShader.use();
            mImageShader.draw();
        }
    }

    /* draw point cloud */ if (mPointCloudPtr) {
        glClear(GL_DEPTH_BUFFER_BIT);
        mPointShader.use();
        mViewMat[1][1] = mViewMat[2][2] = -1;
        mPointShader.setMat4("Model", mModelMat);
        mPointShader.setMat4("View",  mViewMat);
        mPointShader.setMat4("Proj",  mProjMat);
        mPointShader.draw();
    }

}
