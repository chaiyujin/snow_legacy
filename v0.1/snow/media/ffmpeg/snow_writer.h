#pragma once
#include "ffmpeg_head.h"
#include <string>
#include <vector>
#include "../../core/snow_core.h"

namespace snow {

struct OutputStream {
    AVStream *          mStreamPtr;
    AVCodecContext *    mEncCtxPtr;
    int64_t             mNextPts;
    int                 mSamplesCount;
    AVFrame *           mFramePtr;
    AVFrame *           mTmpFramePtr;

    SwsContext *        mSwsCtxPtr;
    SwrContext *        mSwrCtxPtr;

    int                 mWidth, mHeight;
    int                 mSampleRate;

    OutputStream() : mStreamPtr(nullptr), mEncCtxPtr(nullptr)
                   , mNextPts(0), mSamplesCount(0)
                   , mFramePtr(nullptr), mTmpFramePtr(nullptr)
                   , mSwsCtxPtr(nullptr), mSwrCtxPtr(nullptr)
                   , mWidth(0), mHeight(0), mSampleRate(0) {}

    ~OutputStream() {}

    void free() {
        if (mEncCtxPtr)     avcodec_free_context(&mEncCtxPtr);
        if (mFramePtr)      av_frame_free(&mFramePtr);
        if (mTmpFramePtr)   av_frame_free(&mTmpFramePtr);
        if (mSwsCtxPtr)     sws_freeContext(mSwsCtxPtr);
        if (mSwrCtxPtr)     swr_free(&mSwrCtxPtr);
        mEncCtxPtr      = nullptr;
        mFramePtr       = nullptr;
        mTmpFramePtr    = nullptr;
        mSwsCtxPtr      = nullptr;
        mSwrCtxPtr      = nullptr;
    }
};

class MediaWriter {
private:
    std::string                 mFilename;
    AVFormatContext *           mFmtCtxPtr;
    AVCodec *                   mAudioCodecPtr;
    AVCodec *                   mVideoCodecPtr;
    OutputStream *              mAudioStreamPtr;
    OutputStream *              mVideoStreamPtr;
    // audio data
    std::vector<float>          mAudio;
    int                         mSampleIndex;
    // video data
    // encode bool
    bool                        mEncodeVideo;
    bool                        mEncodeAudio;

    void openAudio();
    void openVideo(int bpp);
    bool writeAudioFrame();
    bool writeVideoFrame(const snow::Image *image);
    void resample(int sampleRate);

public:
    // output sample rate is 22050
    static int  AudioSampleRate;
    static int  AudioBitRate;
    static int  VideoBitRate;

    static void setAudioSampleRate(int sr) { AudioSampleRate = sr;  }
    static void setAudioBitRate(int br)    { AudioBitRate = br;     }
    static void setVideoBitRate(int br)    { VideoBitRate = br;     }

    MediaWriter(std::string filename);

    void addAudioStream();
    void addVideoStream(int w, int h, int bpp, int fps);

    /* audio data will be resampled to 22050 */
    void setAudioData(const std::vector<float> &audio, int sampleRate);
    void setAudioData(const std::vector<int16_t> &audio, int sampleRate);

    void appendImage(const snow::Image &image);

    void start(bool dumpFormat=false);
    void finish();
};

}