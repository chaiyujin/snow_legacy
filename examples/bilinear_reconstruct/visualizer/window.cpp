#include "window.h"
#include "../tools/projection.h"
#include "../tools/contour.h"

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
        glm::mat4 viewMat = mCamera.viewMatrix() * mViewMat;

        {
            std::vector<snow::float3> cands;
            std::vector<size_t> candsIndex;
            for (size_t i = 0; i < FaceDB::Contours().size(); ++i) {
                for (size_t j = 0; j < FaceDB::Contours()[i].size(); ++j) {
                    int idx = FaceDB::Contours()[i][j];
                    cands.push_back({
                        mModelShaderPtr->pointsPtr()[idx * 8],
                        mModelShaderPtr->pointsPtr()[idx * 8+1],
                        mModelShaderPtr->pointsPtr()[idx * 8+2]
                    });
                    if (i < FaceDB::Contours().size() - 1) {
                        candsIndex.push_back(idx);
                    }
                }
            }
            
            std::vector<snow::float2> landmarks;
            projectToImageSpace(cands, mProjMat, viewMat, mModelMat, landmarks);
            auto contour_pair = getContourGrahamScan(landmarks);
            set2DLandmarks(landmarks);
        }

        // image
        if (mImageShaderPtr && mShowImage) { glClear(GL_DEPTH_BUFFER_BIT); mImageShaderPtr->use(); mImageShaderPtr->draw(); }
        // point
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (mPointShaderPtr && mShowPoint) {
            mPointShaderPtr->use();
            mPointShaderPtr->setMat4("Model", mModelMat);
            mPointShaderPtr->setMat4("View",  viewMat);
            mPointShaderPtr->setMat4("Proj",  mProjMat);
            mPointShaderPtr->draw();
        }
        // model
        if (mModelShaderPtr && mShowModel) {
            mModelShaderPtr->use();
            mModelShaderPtr->setMat4("Normal",   glm::transpose(glm::inverse(mModelMat)));
            mModelShaderPtr->setMat4("Model",    mModelMat);
            mModelShaderPtr->setMat4("View",     viewMat);
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
