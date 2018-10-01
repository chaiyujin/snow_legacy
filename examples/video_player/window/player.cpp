#include "player.h"

static void writeFrameBin(const char *filename, const uint8_t *color, const uint8_t *depth,
                     int colorSize, int depthSize) {
    FILE *fp = fopen(filename, "wb");
    fwrite(color, 1, colorSize, fp);
    fwrite(depth, 1, depthSize, fp);
    fclose(fp);
}

bool PlayerWindow::openVideo(const std::string filename) {
    MediaReader::initializeFFmpeg();
    // try to open a new video
    closeVideo();
    mReaderPtr = new MediaReader(filename);
    mReaderPtr->setDstAudioSampleRate(16000);
    if (!mReaderPtr->open()) return false;
    mVideoStreamPtr = std::dynamic_pointer_cast<MediaStream> (mReaderPtr->getStreams()[0]);
    mDepthStreamPtr = std::dynamic_pointer_cast<MediaStream> (mReaderPtr->getStreams()[1]);
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
            mDepthStreamPtr->readFrame();
            const FrameBase *frame = mVideoStreamPtr->framePtr();
            this->updateFrame(*(const VideoFrame*)frame);
            {
                const VideoFrame *colorFrame = (const VideoFrame *)mVideoStreamPtr->framePtr();
                const VideoFrame *depthFrame = (const VideoFrame *)mDepthStreamPtr->framePtr();
                writeFrameBin("../../../assets/test_depth/frame.bin",
                              colorFrame->data(), depthFrame->data(),
                              colorFrame->mWidth * colorFrame->mHeight * 4,
                              depthFrame->mWidth * depthFrame->mHeight * 2);
            }
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