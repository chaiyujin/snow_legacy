#include "window.h"

void VisualizerWindow::setImage(const Image &image) {
    if (mImageShaderPtr == nullptr) mImageShaderPtr = new ImageShader();
    mImageShaderPtr->uploadImage(
        image.data(), image.width(), image.height(),
        (image.bpp() == 4) ? GL_RGBA : GL_RGB
    );
}

void VisualizerWindow::set2DLandmarks(const std::vector<snow::float2> &landmarks) {
    if (mLandsShaderPtr == nullptr
        || landmarks.size() != mLandsShaderPtr->numPoints()) {
        delete mLandsShaderPtr;
        mLandsShaderPtr = new LandmarksShader(landmarks.size());
    }
    mLandsShaderPtr->update2DLandmarks(landmarks);
}

void VisualizerWindow::set3DLandmarks(const std::vector<snow::float3> &landmarks) {
    if (mLandsShaderPtr == nullptr
        || landmarks.size() != mLandsShaderPtr->numPoints()) {
        delete mLandsShaderPtr;
        mLandsShaderPtr = new LandmarksShader(landmarks.size());
    }
    mLandsShaderPtr->update3DLandmarks(landmarks);
}

void VisualizerWindow::setPointCloud(const PointCloud &pointCloud) {
    if (mPointShaderPtr == nullptr
        || mPointShaderPtr->numPoints() != pointCloud.numPoints()) {
        delete mPointShaderPtr;
        mPointShaderPtr = new PointCloudShader(pointCloud.numPoints(), mImageShaderPtr);
    }
    mPointShaderPtr->setTextureShader(mImageShaderPtr);
    mPointShaderPtr->updateWithPointCloud(pointCloud);
}

void VisualizerWindow::setMorphModel(const MorphModel &morphModel) {
    if (mModelShaderPtr == nullptr
        || mModelShaderPtr->numVertices() != morphModel.numVertices()
        || mModelShaderPtr->numTriangles() != morphModel.numTriangles()) {
        delete mModelShaderPtr;
        mModelShaderPtr = new MorphModelShader(morphModel.numVertices(), morphModel.numTriangles());
    }
    mModelShaderPtr->updateWithMorphModel(morphModel);
}

void VisualizerWindow::processEvent(SDL_Event &event) {
    mCamera.processMouseEvent(event);
}

void VisualizerWindow::draw() {
    /* ui */ {
        ImGui::Begin("tools");
        if (ImGui::Button("reset camera")) mCamera.reset();
        if (mImageShaderPtr) { ImGui::Checkbox("image",       &mShowImage); }
        if (mImageShaderPtr && mLandsShaderPtr) ImGui::SameLine();
        if (mLandsShaderPtr) { ImGui::Checkbox("landmarks",   &mShowLands); }
        if (mPointShaderPtr) { ImGui::Checkbox("point cloud", &mShowPoint); }
        if (mModelShaderPtr) { ImGui::Checkbox("model",       &mShowModel); }
        ImGui::End();
    }

    /* drawing */ {
        mViewMat = mCamera.viewMatrix();
        // image
        if (mImageShaderPtr && mShowImage) { glClear(GL_DEPTH_BUFFER_BIT); mImageShaderPtr->use(); mImageShaderPtr->draw(); }
        // point
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (mPointShaderPtr && mShowPoint) {
            mPointShaderPtr->use();
            mPointShaderPtr->setMat4("Model", mModelMat);
            mPointShaderPtr->setMat4("View",  mViewMat);
            mPointShaderPtr->setMat4("Proj",  mProjMat);
            mPointShaderPtr->draw();
        }
        // model
        if (mModelShaderPtr && mShowModel) {
            mModelShaderPtr->use();
            mModelShaderPtr->setMat4("Normal",   glm::transpose(glm::inverse(mModelMat)));
            mModelShaderPtr->setMat4("Model",    mModelMat);
            mModelShaderPtr->setMat4("View",     mViewMat);
            mModelShaderPtr->setMat4("Proj",     mProjMat);
            mModelShaderPtr->setVec3("LightPos", glm::vec3(0.0f, 0.0f, -10.0f));
            mModelShaderPtr->setVec3("Ambient",  glm::vec3(0.1f, 0.1f, 0.1f));
            mModelShaderPtr->setVec3("Diffuse",  glm::vec3(0.7f, 0.7f, 0.7f));
            mModelShaderPtr->draw();
        }
        // landmarks
        if (mLandsShaderPtr && mShowLands) { glClear(GL_DEPTH_BUFFER_BIT); mLandsShaderPtr->use(); mLandsShaderPtr->draw(); }
    }
}
