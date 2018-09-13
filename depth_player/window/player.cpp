#include "player.h"

bool PlayerWindow::openVideo(const std::string filename) {
    DepthVideoReader::initialize_ffmpeg();
    // try to open a new video
    closeVideo();
    mReaderPtr = new DepthVideoReader(filename);
    if (!mReaderPtr->open()) return false;

    /* first frame */ {
        seek();
        mReaderPtr->seek(0);
    }

    return true;
}

void PlayerWindow::closeVideo() {
    if (mReaderPtr) delete mReaderPtr;
    mReaderPtr = nullptr;
    mCurrentTime = mPlayerSecond = 0;
}

void PlayerWindow::updateFrame(const VideoFrame &frame) {
    mImageShader.uploadImage(frame.mData.get(), frame.mWidth, frame.mHeight, GL_RGBA);
    mCurrentTime = (float)frame.mTimestamp;
    mPlayerSecond = (float) (mCurrentTime / 1000.0);
}

void PlayerWindow::seek() {
    if (mReaderPtr) {
        mReaderPtr->seek(mPlayerSecond * 1000.0);
        auto framepair = mReaderPtr->read_frame_pair();
        this->updateFrame(framepair.first);
    }
}

void PlayerWindow::draw() {
    if (mReaderPtr == nullptr) {
        ImGui::Text("Please open a video.");
        return;
    }

    mImageShader.use();
    mImageShader.draw();

    /* draw ui */ {
        ImGui::Begin("player");
        if (ImGui::SliderFloat("seconds", &mPlayerSecond, 0, mReaderPtr->duration_ms() / 1000.0))
            this->seek();
        ImGui::End();
    }
}