#pragma once
#include "ffmpeg_header.h"

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
        mTmpFramePtr    = av_frame_alloc();
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

class DepthVideoReader {
private:
    static bool gInitialized;
    
public:

};