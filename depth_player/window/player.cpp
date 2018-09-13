#include "player.h"

bool PlayerWindow::openVideo(const std::string filename) {
    DepthVideoReader::initialize_ffmpeg();
    // try to open a new video
    closeVideo();
    mStreamPtr = new StreamBase(0, MediaType::Video);
    mReaderPtr = new DepthVideoReader(filename);
    mStreamPtr->setInput(mReaderPtr);
    if (!mReaderPtr->open()) return false;

    /* first frame */ {
        seek();
        mStreamPtr->seek(0);
    }

    return true;
}

void PlayerWindow::closeVideo() {
    if (mReaderPtr) delete mReaderPtr;
    if (mStreamPtr) delete mStreamPtr;
    mStreamPtr = nullptr;
    mReaderPtr = nullptr;
    mCurrentTime = mPlayerSecond = 0;
}

void PlayerWindow::updateFrame(const VideoFrame &frame) {
    mImageShader.uploadImage(frame.mData.get(), frame.mWidth, frame.mHeight, GL_RGBA);
    mCurrentTime = (float)frame.mTimeStamp;
    mPlayerSecond = (float) (mCurrentTime / 1000.0);
}

void PlayerWindow::seek() {
    if (mStreamPtr) {
        mStreamPtr->seek(mPlayerSecond * 1000.0);
        if (mStreamPtr->readFrame()) {
            FrameBase *frame = mStreamPtr->frame();
            this->updateFrame(*(VideoFrame*)frame);
        }
        else {
            printf("fuck\n");
        }
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