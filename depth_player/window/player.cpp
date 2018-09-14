#include "player.h"

bool PlayerWindow::openVideo(const std::string filename) {
    MediaReader::initialize_ffmpeg();
    // try to open a new video
    closeVideo();
    printf("close\n");
    mReaderPtr = new MediaReader(filename);
    printf("new media\n");
    if (!mReaderPtr->open()) return false;
    printf("open\n");
    mVideoStreamPtr = std::dynamic_pointer_cast<MediaStream> (mReaderPtr->getStreams()[0]);
    printf("get streams %d %d\n", mVideoStreamPtr->videoFormat().mWidth, mVideoStreamPtr->videoFormat().mHeight);

    /* first frame */ {
        seek();
        mVideoStreamPtr->seek(0);
    }

    return true;
}

void PlayerWindow::closeVideo() {
    if (mReaderPtr) delete mReaderPtr;
    mReaderPtr = nullptr;
    mCurrentTime = mPlayerSecond = 0;
    {
        mVideoStreamPtr.reset();
        mDepthStreamPtr.reset();
        mAudioStreamPtrList.clear();
    }
}

void PlayerWindow::updateFrame(const VideoFrame &frame) {
    mImageShader.uploadImage(frame.data(), frame.mWidth, frame.mHeight, GL_RGBA);
    mCurrentTime = (float)frame.timestamp();
    mPlayerSecond = (float) (mCurrentTime / 1000.0);
}

void PlayerWindow::seek() {
    if (mVideoStreamPtr) {
        mVideoStreamPtr->seek(mPlayerSecond * 1000.0);
        if (mVideoStreamPtr->readFrame()) {
            const FrameBase *frame = mVideoStreamPtr->framePtr();
            this->updateFrame(*(const VideoFrame*)frame);
        }
        else {
            printf("end of video\n");
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
        if (ImGui::SliderFloat("seconds", &mPlayerSecond, 0, mReaderPtr->duration() / 1000.0))
            this->seek();
        ImGui::End();
    }
}