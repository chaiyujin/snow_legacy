#pragma once
#define SNOW_MODULE_OPENGL
#include "../shaders/image.h"
#include "../depth_source/data.h"
#include <snow.h>

class ShowWindow : public snow::AbstractWindow {
    const Image *       mColorPtr;
    const Image *       mDepthPtr;
    Image *             mColorizedDepth;
    const PointCloud *  mPointCloudPtr;
    ImageShader         mImageShader;

public:
    ShowWindow() : AbstractWindow()
                 , mColorPtr(nullptr)
                 , mDepthPtr(nullptr)
                 , mColorizedDepth(nullptr)
                 , mPointCloudPtr(nullptr) {}
    ~ShowWindow() { delete mColorPtr; delete mDepthPtr; delete mColorizedDepth; delete mPointCloudPtr; }

    void setColor(const Image *imgPtr);
    void setDepth(const Image *depthPtr);
    
    void processEvent(SDL_Event &event) {}
    void draw() {
        if (mColorPtr) {
            mImageShader.use();
            mImageShader.draw();
        }
    }
};
