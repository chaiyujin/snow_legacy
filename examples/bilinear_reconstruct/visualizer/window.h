#pragma once
#include "../depth_source/data.h"
#include "../shaders/image.h"
#include "../shaders/point_cloud.h"
#include "../shaders/morph_model.h"
#include <snow.h>

class VisualizerWindow : public snow::AbstractWindow {
public:
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
    /* mat */
    glm::mat4           mViewMat;
    glm::mat4           mProjMat;
    glm::mat4           mModelMat;

    void updateImageWithColor();
    void updateImageWithDepth();
    void updatePointCloud();

    VisualizerWindow(int numLandmarks=75, int numPoints=640*480, const char *title="show")
        : AbstractWindow(title)
        , mColorPtr(nullptr) , mDepthPtr(nullptr) , mPointCloudPtr(nullptr), mModelPtr(nullptr)
        , mImageShader(numLandmarks), mPointShader(numPoints, &mImageShader)
        , mModelShader(11510, 22800)
        , mShowColor(true), mShowDepth(false), mShowPoint(true)
        , mViewMat(1.0), mProjMat(1.0), mModelMat(1.0) {}
    ~VisualizerWindow() {}
    /*set data*/
    void setColor(const Image *imgPtr);
    void setDepth(const Image *depthPtr);
    void setPointCloud(const PointCloud *pointCloudPtr);

    /*set mat*/
    void setModelMat(const glm::mat4 &model) { mModelMat = model; }
    void setViewMat(const glm::mat4 &view)   { mViewMat = view;   }
    void setProjMat(const glm::mat4 &proj)   { mProjMat = proj;   }
    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event);
    void draw();
};
