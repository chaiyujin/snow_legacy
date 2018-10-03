#pragma once
#include "ffmpeg_head.h"
#include <string>
#include <vector>

namespace snow {

class MediaWriter;
class OutputStream {
    friend class MediaWriter;

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

public:
    OutputStream() : mStreamPtr(nullptr), mEncCtxPtr(nullptr)
                   , mNextPts(0), mSamplesCount(0)
                   , mFramePtr(nullptr), mTmpFramePtr(nullptr)
                   , mSwsCtxPtr(nullptr), mSwrCtxPtr(nullptr)
                   , mWidth(0), mHeight(0), mSampleRate(0) {}
    ~OutputStream() {
        if (mEncCtxPtr)     avcodec_free_context(&mEncCtxPtr);
        if (mFramePtr)      av_frame_free(&mFramePtr);
        if (mTmpFramePtr)   av_frame_free(&mTmpFramePtr);
        if (mSwsCtxPtr)     { sws_freeContext(mSwsCtxPtr); mSwsCtxPtr == nullptr; }
        if (mSwrCtxPtr)     swr_free(&mSwrCtxPtr);
    }
};

class MediaWriter {
private:
    std::string                 mFilename;
    AVFormatContext *           mFmtCtxPtr;
    AVCodec *                   mAudioCodecPtr;
    AVCodec *                   mVideoCodecPtr;
    OutputStream *              mVideoStreamPtr;
    OutputStream *              mAudioStreamPtr;

    std::vector<float>          mAudio;
    int                         mSampleIndex;
    int                         mSampleRate;

    OutputStream * addStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codecId, int width, int height, int fps, int sampleRate);
    void openAudio();
    bool write_audio_frame();
public:
    MediaWriter(std::string filename);

    void setAudioTrack(const std::vector<float> &audio, int sampleRate);
    void setAudioTrack(const std::vector<int16_t> &audio, int sampleRate);

    void write();
    void close() {

        av_write_trailer(mFmtCtxPtr);

        // /* Close each codec. */
        // if (have_video)
        //     close_stream(oc, &video_st);
        // if (have_audio)
        //     close_stream(oc, &audio_st);

        if (!(mFmtCtxPtr->oformat->flags & AVFMT_NOFILE))
            /* Close the output file. */
            avio_closep(&mFmtCtxPtr->pb);

        /* free the stream */
        avformat_free_context(mFmtCtxPtr);
    }
};

}