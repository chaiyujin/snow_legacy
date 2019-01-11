#pragma once
#include "../depth_source/data.h"
#include "shader_image.h"
#include "shader_point.h"
#include "shader_model.h"
#include "shader_lands.h"
#include <snow.h>

class VisualizerWindow : public snow::AbstractWindow {
    snow::ArcballCamera mCamera;
    ImageShader *       mImageShaderPtr;
    LandmarksShader *   mLandsShaderPtr;
    PointCloudShader *  mPointShaderPtr;
    MorphModelShader *  mModelShaderPtr;
    /* gui */
    bool                mShowImage;
    bool                mShowLands;
    bool                mShowPoint;
    bool                mShowModel;
    /* mat */
    glm::mat4           mViewMat;
    glm::mat4           mProjMat;
    glm::mat4           mModelMat;
    /* data */
    int                                     mToShow;
    int                                     mCurrentShow;
    std::vector<Image>                      mImageList;
    std::vector<std::vector<snow::float2>>  mLandmarksList;
    std::vector<PointCloud>                 mPointCloudList;
    std::vector<MorphModel>                 mMorphModelList;
    std::vector<glm::mat4>                  mViewMatList;
    std::vector<glm::mat4>                  mProjMatList;
    std::vector<glm::mat4>                  mModelMatList;

    /*set data*/
    void setImage(const Image &image);
    void setPointCloud(const PointCloud &pointCloud);
    void setMorphModel(const MorphModel &morphModel);
    void set2DLandmarks(const std::vector<snow::float2> &landmarks);
    void set3DLandmarks(const std::vector<snow::float3> &landmarks);

    /*set mat*/
    void setModelMat(const glm::mat4 &model) { mModelMat = model; }
    void setViewMat(const glm::mat4 &view)   { mViewMat = view;   }
    void setProjMat(const glm::mat4 &proj)   { mProjMat = proj;   }

public:

    VisualizerWindow(int numLandmarks=75,   int numPoints=640*480,
                     int numVertices=11510, int numTriangles=22800,
                     glm::vec3 eye   =glm::vec3(0.f, 0.f, 0.f),
                     glm::vec3 up    =glm::vec3(0.f, 1.f, 0.f),
                     glm::vec3 lookAt=glm::vec3(0.f, 0.f, -0.5f),
                     const char *title="show")
        : AbstractWindow(title)
        , mCamera(eye, up, lookAt)  // default camera view is identity
        , mImageShaderPtr(nullptr)
        , mLandsShaderPtr(nullptr)
        , mPointShaderPtr(nullptr)
        , mModelShaderPtr(nullptr)
        , mShowImage(true), mShowLands(true), mShowPoint(true), mShowModel(true)
        , mViewMat(1.0), mProjMat(1.0), mModelMat(1.0)
        , mToShow(0), mCurrentShow(-1) {
        mCamera.setSpeedMove(1.f);
        mCamera.setSpeedRotate(0.5f);
    }
    ~VisualizerWindow() {
        delete mImageShaderPtr; mImageShaderPtr = nullptr;
        delete mLandsShaderPtr; mLandsShaderPtr = nullptr;
        delete mPointShaderPtr; mPointShaderPtr = nullptr;
        delete mModelShaderPtr; mModelShaderPtr = nullptr;
    }

    /* append data */
    void appendImage(const Image &image)                                { mImageList.push_back(image);              }
    void appendPointCloud(const PointCloud &pointCloud)                 { mPointCloudList.push_back(pointCloud);    }
    void appendMorphModel(const MorphModel &morphModel)                 { mMorphModelList.push_back(morphModel);    }
    void append2DLandmarks(const std::vector<snow::float2> &landmarks)  { mLandmarksList.push_back(landmarks);      }

    void appendModelMat(const glm::mat4 &model)                         { mModelMatList.push_back(model); }
    void appendViewMat(const glm::mat4 &view)                           { mViewMatList.push_back(view);   }
    void appendProjMat(const glm::mat4 &proj)                           { mProjMatList.push_back(proj);   }

    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event);
    void draw();
};
