#pragma once
#include "../depth_source/data.h"
#include "shader_image.h"
#include "shader_point.h"
#include "shader_model.h"
#include <snow.h>

class VisualizerWindow : public snow::AbstractWindow {
    snow::ArcballCamera mCamera;
    const Image *       mColorPtr;
    const Image *       mDepthPtr;
    const PointCloud *  mPointCloudPtr;
    const MorphModel *  mModelPtr;
    Image               mColorizedDepth;
    ImageShader         mImageShader;
    PointCloudShader    mPointShader;
    MorphModelShader    mModelShader;
    /* gui */
    bool                mShowColor;
    bool                mShowDepth;
    bool                mShowPoint;
    bool                mShowModel;
    /* mat */
    glm::mat4           mViewMat;
    glm::mat4           mProjMat;
    glm::mat4           mModelMat;

public:

    VisualizerWindow(int numLandmarks=75,   int numPoints=640*480,
                     int numVertices=11510, int numTriangles=22800,
                     glm::vec3 eye   =glm::vec3(0.f, 0.f, 0.f),
                     glm::vec3 up    =glm::vec3(0.f, -1.f, 0.f),
                     glm::vec3 lookAt=glm::vec3(0.f, 0.f, 1.f),
                     const char *title="show")
        : AbstractWindow(title)
        , mCamera(eye, up, lookAt)
        , mColorPtr(nullptr) , mDepthPtr(nullptr) , mPointCloudPtr(nullptr), mModelPtr(nullptr)
        , mImageShader(numLandmarks), mPointShader(numPoints, &mImageShader)
        , mModelShader(numVertices, numTriangles)
        , mShowColor(true), mShowDepth(false), mShowPoint(true), mShowModel(true)
        , mViewMat(1.0), mProjMat(1.0), mModelMat(1.0) {
        mCamera.setSpeedMove(1.f);
        mCamera.setSpeedRotate(0.5f);
    }
    ~VisualizerWindow() {}
    /*set data*/
    void setColor(const Image *imgPtr);
    void setDepth(const Image *depthPtr);
    void setPointCloud(const PointCloud *pointCloudPtr);
    void setMorphModel(const MorphModel *morphModel);

    /*set mat*/
    void setModelMat(const glm::mat4 &model) { mModelMat = model; }
    void setProjMat(const glm::mat4 &proj)   { mProjMat = proj;   }

    void updateImageWithColor();
    void updateImageWithDepth();
    void updatePointCloud();
    void updateMorphModel();

    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event);
    void draw();
};
