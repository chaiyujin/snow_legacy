#pragma once
#include "ffmpeg_head.h"
#include "../core/snow_core.h"

namespace snow {

struct AudioFormat {
    AVSampleFormat	mSampleFmt;
    uint64_t		mChLayout;
    uint16_t		mChannels;
    uint32_t		mSampleRate;

    AudioFormat(AVSampleFormat sampleFmt = AV_SAMPLE_FMT_S16,
        uint64_t chLayout = AV_CH_LAYOUT_MONO,
        uint16_t channels = 1, uint32_t sampleRate = 44100)
        : mSampleFmt(sampleFmt)
        , mChLayout(chLayout)
        , mChannels(channels)
        , mSampleRate(sampleRate)
    {}
};

struct InputStream {
    AVMediaType		mType;
    AVCodec	*		mDecPtr;
    AVCodecContext *mDecCtxPtr;
    AVStream *		mStreamPtr;
    /* -- pre alloc frames -- */
    AVFrame	*		mDecodeFramePtr;
    AVFrame *		mTmpFramePtr;
    SwsContext *	mSwsCtxPtr;        // video convert context
    SwrContext *	mSwrCtxPtr;        // audio convert context
    /* -- sync timestamps -- */
    int64_t			mStart;			/* time when read started */
    int64_t			mNextDts;		/* predicted dts of the next packet read for this stream or
                                    (when there are several frames in a packet) of the next
                                    frame in current packet (in AV_TIME_BASE units) */
    int64_t			mDts;			///< dts of the last packet read for this stream (in AV_TIME_BASE units)
    int64_t			mNextPts;		///< synthetic pts for the next decode frame (in AV_TIME_BASE units)
    int64_t			mPts;			///< current pts of the decoded frame  (in AV_TIME_BASE units)
    int64_t			mMinPts, mMaxPts;
    int				mNumSamples;

    int				mVideoStreamIdx;  // stream index in all video streams
    int				mAudioStreamIdx;  // stream index in all audio streams

    InputStream() : mDecPtr(NULL), mDecCtxPtr(NULL)
                  , mDecodeFramePtr(NULL), mTmpFramePtr(NULL)
                  , mSwsCtxPtr(NULL), mSwrCtxPtr(NULL)
                  , mStart(AV_NOPTS_VALUE)
                  , mNextDts(AV_NOPTS_VALUE), mDts(AV_NOPTS_VALUE)
                  , mNextPts(AV_NOPTS_VALUE), mPts(AV_NOPTS_VALUE)
                  , mVideoStreamIdx(-1), mAudioStreamIdx(-1) {
        mDecodeFramePtr = av_frame_alloc();
    }

    ~InputStream() {
        if (mDecodeFramePtr) av_frame_free(&mDecodeFramePtr);
        if (mTmpFramePtr)    av_frame_free(&mTmpFramePtr);
        if (mDecCtxPtr) avcodec_free_context(&mDecCtxPtr);
        if (mSwsCtxPtr) { sws_freeContext(mSwsCtxPtr); mSwsCtxPtr = NULL; }
        if (mSwrCtxPtr) swr_free(&mSwrCtxPtr);
    }

    bool is_depth() { return strcmp(mDecPtr->name, "librvldepth") == 0; }
};


class VideoFrame : public FrameBase {
public:
    int     mWidth, mHeight;
    bool    mIsDepth;
    VideoFrame(): mWidth(0), mHeight(0), mIsDepth(false) {}
    bool isNull() { return mWidth == 0 || mHeight == 0 || mTimeStamp == SNOW_NONE_PTS; }
    void fromAVFrame(AVFrame *frame, int64_t pts, bool depth) {
        mType       = MediaType::Video;
        mWidth      = frame->width;
        mHeight     = frame->height;
        mTimeStamp  = pts;
        mIsDepth    = depth;
        mDataSize   = frame->width * frame->height * 2;    // uint16_t
        if (!mIsDepth) mDataSize *= 2;                       // rgba32
        mData.reset(new uint8_t[mDataSize]);
        memcpy(mData.get(), frame->data[0], mDataSize);
    }
};

class AudioFrame : public FrameBase {
public:
    int     mNunSamples;
    int	    mBytePerSample;
    AudioFrame() :  mNunSamples(0) {}
    bool isNull() const { return mNunSamples == 0 || mTimeStamp == SNOW_NONE_PTS; }
    void fromAVFrame(AVFrame *frame, int64_t pts, int nb_samples, int byte_per_sample) {
        mType           = MediaType::Audio;
        mNunSamples     = nb_samples;
        mBytePerSample  = byte_per_sample;
        mTimeStamp      = pts;
        mDataSize       = nb_samples * byte_per_sample;
        mData.reset(new uint8_t[mDataSize]);
        memcpy(mData.get(), frame->data[0], mDataSize);
    }
};

class DepthVideoReader : public InputBase {
private:
    static bool                 gInitialized;
    static snow::color_map      gJetCmap;
    std::string                 mFilename;
    std::vector<InputStream *>  mStreamPtrList;
    AVFormatContext	*			mFmtCtxPtr;
    int64_t						mStartTime;
	int64_t						mInputTsOffset;
	int64_t						mTsOffset;
	int64_t						mDuration;
	AVRational					mTimeBase;
	bool						mErrorAgain;
	AudioFormat					mDstAudioFmt;

	std::vector<SafeQueue<VideoFrame>*>   mVideoQueues;
	std::vector<SafeQueue<AudioFrame>*>   mAudioQueues;

    // mutex for format context ptr
    std::mutex                  mFmtCtxMutex;

public:
    static void initialize_ffmpeg() { if (!gInitialized) { av_register_all(); gInitialized = true; } }

    DepthVideoReader(const std::string &filename);
    DepthVideoReader(const DepthVideoReader &b);
    ~DepthVideoReader();

    bool    open();
    void    close();
    int     process_input(MediaType request_type=MediaType::AudioVideo);
    int64_t duration_ms();
    void    seek(int64_t ms);

    FrameBase * readFrame(int64_t id, MediaType type);

    // read data
    std::pair<VideoFrame, VideoFrame> read_frame_pair();

    static void colorize(const VideoFrame &frame, VideoFrame &colorized) {
        int pixels = frame.mWidth * frame.mHeight;
        colorized.mWidth = frame.mWidth;
        colorized.mHeight = frame.mHeight;
        colorized.mData.reset(new uint8_t[pixels * 4]);
        uint8_t *output = colorized.mData.get();
        memset(output, 0, pixels * 4);

        int hist[65536];
        memset(hist, 0, sizeof(hist));
        uint16_t *depth = (uint16_t *)frame.mData.get();
        for (int i = 0; i < pixels; ++i) {
            if (!depth[i]) continue;
            ++hist[depth[i]];
        }
        for (int i = 1; i < 65536; ++i) hist[i] += hist[i - 1];

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < pixels; ++i) {
            if (depth[i] == 0) continue;
            float val = (float)hist[depth[i]] / hist[0xFFFF];
            auto v = gJetCmap.get(val);
            output[i * 4] = (uint8_t)v.x;
            output[i * 4 + 1] = (uint8_t)v.y;
            output[i * 4 + 2] = (uint8_t)v.z;
            output[i * 4 + 3] = 255;
        }
    }
};

}