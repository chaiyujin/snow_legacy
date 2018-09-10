#pragma once
#include "ffmpeg_header.h"
#include <map>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>
#include <condition_variable>
#include <snow.h>

template <class T>
class StreamQueue {
private:
    size_t                  mSize;
    std::queue<T>           mQueue;
    mutable std::mutex      mMutex;
    std::condition_variable mCondVar;

public:
    StreamQueue(void) : mQueue(), mMutex(), mCondVar(), mSize(0) {}
    ~StreamQueue(void) {}

    // Add an element to the queue.
    void push(T t) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(t);
        mSize++;
        mCondVar.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    void pop(void) {
        std::unique_lock<std::mutex> lock(mMutex);
        // release lock as long as the wait and reaquire it afterwards.
        while (mQueue.empty())
            mCondVar.wait(lock);
        mQueue.pop();
        mSize--;
    }

    T &front() {
        std::unique_lock<std::mutex> lock(mMutex);
        // release lock as long as the wait and reaquire it afterwards.
        while (mQueue.empty())
            mCondVar.wait(lock);
        return mQueue.front();
    }

    size_t size() const { return mSize; }

};

AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples);
AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);

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

struct AudioFormat {
    AVSampleFormat	sample_fmt;
    uint64_t		ch_layout;
    uint16_t		channels;
    uint32_t		sample_rate;

    AudioFormat(AVSampleFormat sample_fmt = AV_SAMPLE_FMT_S16,
        uint64_t ch_layout = AV_CH_LAYOUT_MONO,
        uint16_t channels = 1, uint32_t sample_rate = 44100)
        : sample_fmt(sample_fmt)
        , ch_layout(ch_layout)
        , channels(channels)
        , sample_rate(sample_rate)
    {}
};

class VideoFrame {
public:
    std::shared_ptr<uint8_t>    mData;
    int                         mWidth, mHeight;
    bool                        mIsDepth;
    double                      mTimestamp;
    VideoFrame(): mWidth(0), mHeight(0), mIsDepth(false), mTimestamp(std::numeric_limits<double>::max()) {}
    bool isNull() { return mWidth == 0 || mHeight == 0 || mTimestamp == std::numeric_limits<double>::max(); }
    void fromAVFrame(AVFrame *frame, double pts, bool depth) {
        mWidth      = frame->width;
        mHeight     = frame->height;
        mTimestamp  = pts;
        mIsDepth    = depth;
        int size = frame->width * frame->height * 2;    // uint16_t
        if (!mIsDepth) size *= 2;                       // rgba32
        mData.reset(new uint8_t[size]);
        memcpy(mData.get(), frame->data[0], size);
    }
};

class AudioFrame {
public:
    std::shared_ptr<uint8_t> mData;
    int			             mNunSamples;
    int			             mBytePerSample;
    double		             mTimestamp;
    AudioFrame() : mNunSamples(0), mTimestamp(std::numeric_limits<double>::max()) {}
    bool isNull() const { return mNunSamples == 0 || mTimestamp == std::numeric_limits<double>::max(); }
    void fromAVFrame(AVFrame *frame, double pts, int nb_samples, int byte_per_sample) {
        mNunSamples     = nb_samples;
        mBytePerSample  = byte_per_sample;
        mTimestamp      = pts;
        mData.reset(new uint8_t[nb_samples * byte_per_sample]);
        memcpy(mData.get(), frame->data[0], nb_samples * byte_per_sample);
    }
};

class DepthVideoReader {
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

	std::vector<StreamQueue<VideoFrame>*>   mVideoQueues;
	std::vector<StreamQueue<AudioFrame>*>   mAudioQueues;

    // mutex for format context ptr
    std::mutex                  mFmtCtxMutex;

public:
    static void initialize_ffmpeg() { if (!gInitialized) { av_register_all(); gInitialized = true; } }
    enum Type { Audio = 1, Video = 2, All = 3 };

    DepthVideoReader(const std::string &filename);
    ~DepthVideoReader();

    bool    open();
    void    close();
    int     process_input(Type request_type=Type::All);
    int64_t duration_ms();
    void    seek(int64_t ms);

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