#pragma once
#include <snow.h>
#include <memory>
#include "../shader/image.h"
using namespace snow;

class PlayerWindow : public snow::AbstractWindow {
private:
    std::shared_ptr<MediaStream> mVideoStreamPtr;
    std::shared_ptr<MediaStream> mDepthStreamPtr;
    std::vector<std::shared_ptr<MediaStream>> mAudioStreamPtrList;
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
        , mVideoStreamPtr(nullptr)
        , mDepthStreamPtr(nullptr)
        , mAudioStreamPtrList(0)
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