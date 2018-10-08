#include "window.h"
#include "../tools/projection.h"
#include "../tools/contour.h"
#include "../tools/math_tools.h"

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
        mLandsShaderPtr = new LandmarksShader((int)landmarks.size());
    }
    mLandsShaderPtr->update2DLandmarks(landmarks);
}

void VisualizerWindow::set3DLandmarks(const std::vector<snow::float3> &landmarks) {
    if (mLandsShaderPtr == nullptr
        || landmarks.size() != mLandsShaderPtr->numPoints()) {
        delete mLandsShaderPtr;
        mLandsShaderPtr = new LandmarksShader((int)landmarks.size());
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
        if (mLandmarksList.size() > 1) {
            ImGui::SliderInt("Frame", &mToShow, 0, (int)mLandmarksList.size() - 1);
        }
        if (mImageShaderPtr) { ImGui::Checkbox("image",       &mShowImage); }
        if (mImageShaderPtr && mLandsShaderPtr) ImGui::SameLine();
        if (mLandsShaderPtr) { ImGui::Checkbox("landmarks",   &mShowLands); }
        if (mPointShaderPtr) { ImGui::Checkbox("point cloud", &mShowPoint); }
        if (mModelShaderPtr) { ImGui::Checkbox("model",       &mShowModel); }
        ImGui::End();
    }

    if (mToShow != mCurrentShow) {
        if (0 <= mToShow && mToShow < mImageList.size())        setImage(mImageList[mToShow]);
        if (0 <= mToShow && mToShow < mLandmarksList.size())    set2DLandmarks(mLandmarksList[mToShow]);
        if (0 <= mToShow && mToShow < mMorphModelList.size())   setMorphModel(mMorphModelList[mToShow]);
        if (0 <= mToShow && mToShow < mPointCloudList.size())   setPointCloud(mPointCloudList[mToShow]);
        if (0 <= mToShow && mToShow < mViewMatList.size())      setViewMat(mViewMatList[mToShow]);
        if (0 <= mToShow && mToShow < mProjMatList.size())      setProjMat(mProjMatList[mToShow]);
        if (0 <= mToShow && mToShow < mModelMatList.size())     setModelMat(mModelMatList[mToShow]);
        mCurrentShow = mToShow;
    }

    /* drawing */ {
        glm::mat4 viewMat = mCamera.viewMatrix() * mViewMat;

        {
            // std::vector<snow::float3> cands;
            // std::vector<size_t> candsIndex;
            // for (size_t i = 0; i < FaceDB::Contours().size(); ++i) {
            //     for (size_t j = 0; j < FaceDB::Contours()[i].size(); ++j) {
            //         int idx = FaceDB::Contours()[i][j];
            //         cands.push_back({
            //             mModelShaderPtr->pointsPtr()[idx * 8],
            //             mModelShaderPtr->pointsPtr()[idx * 8+1],
            //             mModelShaderPtr->pointsPtr()[idx * 8+2]
            //         });
            //         candsIndex.push_back(idx);
            //     }
            // }
            
            // std::vector<snow::float2> cands2d;
            // projectToImageSpace(cands, mProjMat, viewMat, mModelMat, cands2d);
            // auto contour_pair = getContourGrahamScan(cands2d);
            // std::vector<snow::float2> finalContour;
            // {
            //     for (size_t i = 0, k=0; i < FaceDB::Contours().size(); ++i) {
            //         double compare = 0;
            //         int select = -1;
            //         for (size_t j = 0; j < FaceDB::Contours()[i].size(); ++j, ++k) {
            //             int idx = FaceDB::Contours()[i][j];
            //             if (i < 10) {
            //                 if (select < 0 || cands2d[k].x < compare) {
            //                     compare = cands2d[k].x;
            //                     select = k;
            //                 }
            //             }
            //             else if (i >= FaceDB::Contours().size() - 10) {
            //                 if (select < 0 || cands2d[k].x > compare) {
            //                     compare = cands2d[k].x;
            //                     select = k;
            //                 }
            //             }
            //             else {
            //                 for (size_t li = 0; li < contour_pair.first.size() - 1; ++li) {
            //                     double dist = PointToLine2D::sqrDistance(
            //                         cands2d[k].x, cands2d[k].y,
            //                         contour_pair.first[li].x,
            //                         contour_pair.first[li].y,
            //                         contour_pair.first[li+1].x,
            //                         contour_pair.first[li+1].y
            //                     );
            //                     if (select < 0 || dist < compare) {
            //                         compare = dist;
            //                         select = k;
            //                     }
            //                 }
            //             }
            //         }
            //         if (select >= 0) finalContour.push_back(cands2d[select]);
            //     }
            // }
            // std::cout << finalContour.size() << std::endl;
            // set2DLandmarks(finalContour);
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
