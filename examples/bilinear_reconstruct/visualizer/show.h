#pragma once
#define SNOW_MODULE_OPENGL
#include "../depth_source/data.h"
#include "../shaders/image.h"
#include "../shaders/point_cloud.h"
#include <snow.h>

class ShowWindow : public snow::AbstractWindow {
    const Image *       mColorPtr;
    const Image *       mDepthPtr;
    Image               mColorizedDepth;
    const PointCloud *  mPointCloudPtr;
    ImageShader         mImageShader;
    PointCloudShader    mPointShader;
    bool                mShowColor;
    bool                mShowDepth;
    /* mat */
    glm::mat4           mViewMat;
    glm::mat4           mProjMat;
    glm::mat4           mModelMat;

    void updateImageWithColor();
    void updateImageWithDepth();
    void updatePointCloud();

public:
    ShowWindow(int numLandmarks=75,
               int numPoints=640*480,
               const char *title="show")
        : AbstractWindow(title)
        , mColorPtr(nullptr)
        , mDepthPtr(nullptr)
        , mPointCloudPtr(nullptr)
        , mImageShader(numLandmarks)
        , mPointShader(numPoints, &mImageShader)
        , mShowColor(true)
        , mShowDepth(false)
        , mViewMat(1.0)
        , mProjMat(1.0)
        , mModelMat(1.0) {}
    ~ShowWindow() { }
    /*set data*/
    void setColor(const Image *imgPtr);
    void setDepth(const Image *depthPtr);
    void setPointCloud(const PointCloud *pointCloudPtr);
    /*set mat*/
    void setModelMat(const glm::mat4 &model) { mModelMat = model; }
    void setViewMat(const glm::mat4 &view)   { mViewMat = view;   }
    void setProjMat(const glm::mat4 &proj)   { mProjMat = proj;   }
    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event) {}
    void draw();
};
