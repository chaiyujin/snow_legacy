#pragma once
#include <snow.h>
#include <memory>
#include "../shader/image.h"
using namespace snow;

class PlayerWindow : public snow::AbstractWindow {
private:
    std::shared_ptr<MediaStream> mStreamPtr;
    MediaReader         *mReaderPtr;
    ImageShader          mImageShader;
    float                mCurrentTime;
    float                mPlayerSecond;
    void updateFrame(const VideoFrame &frame);
    void seek();
    void nextFrame();

public:
    PlayerWindow(const char *title="player")
        : AbstractWindow(title)
        , mReaderPtr(nullptr)
        , mStreamPtr(nullptr)
        , mCurrentTime(0)
        , mPlayerSecond(0) {}
    ~PlayerWindow() {
        closeVideo();
    }

    ImageShader &imageShader() { return mImageShader; }
    bool openVideo(const std::string filename);
    void closeVideo();

    void processEvent(SDL_Event &event) {}
    void draw();

};