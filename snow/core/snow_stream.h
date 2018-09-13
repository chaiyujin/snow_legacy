#pragma once
#include <string>
#include <memory>
#include <functional>
#define SNOW_NONE_PTS       ((int64_t)UINT64_C(0x8000000000000000))
#define SNOW_FRMAE_NONE_ID  ((int64_t)UINT64_C(0x8000000000000000))

namespace snow {

enum MediaType { None=0, Audio=0x0001, Video=0x0010, AudioVideo=0x0011, Other=0x1000 };

struct FrameBase {
    FrameBase() : mType(MediaType::None), mDataSize(0), mTimeStamp(SNOW_NONE_PTS) {}
    FrameBase(MediaType type, uint8_t *data, size_t size, int64_t ts=0) : mType(type), mData(data), mDataSize(size), mTimeStamp(ts) {}
    FrameBase(const FrameBase &b) : mType(b.mType), mData(b.mData), mDataSize(b.mDataSize), mTimeStamp(b.mTimeStamp) {}
    virtual ~FrameBase() {}

    uint8_t *      data()         { return mData.get(); }
    const uint8_t *data()   const { return mData.get(); }
    size_t         size()   const { return mDataSize;   }
    int64_t        ts()     const { return mTimeStamp; }

    MediaType                   mType;
    std::shared_ptr<uint8_t>    mData;
    size_t                      mDataSize;
    int64_t                     mTimeStamp;
};

struct InputBase {
    virtual bool open() = 0;
    virtual void seek(int64_t ms) = 0;
    virtual FrameBase * readFrame(int64_t id, MediaType type) = 0;
};

class StreamBase {
public:
    StreamBase(): mInputPtr(nullptr), mID(SNOW_FRMAE_NONE_ID), mType(MediaType::None), mFrame(nullptr)
                , mStartTime(SNOW_NONE_PTS), mCurrTime(SNOW_NONE_PTS)
                , mNextTime(SNOW_NONE_PTS), mDuration(0) {}
    StreamBase(int64_t id, MediaType type, int64_t start=0, int64_t duration=0)
                : mInputPtr(nullptr), mID(id), mType(type), mFrame(nullptr)
                , mStartTime(start), mCurrTime(0), mNextTime(start), mDuration(duration) {}
    virtual ~StreamBase() { clearFrame(); }

    void setInput(InputBase *input) { mInputPtr = input; }
    void clearFrame()               { if (mFrame) delete mFrame; mFrame = nullptr; }
    FrameBase *frame()              { return mFrame; }
    
    virtual bool readFrame() {
        clearFrame();
        if (mInputPtr) {
            mFrame = mInputPtr->readFrame(mID, mType);
            return mFrame != nullptr;
        }
        else {
            throw std::runtime_error("Input is not set for stream!\n");
        }
    }
    virtual void seek(int64_t ms) {
        if (mInputPtr)  mInputPtr->seek(ms);
        else            throw std::runtime_error("Input is not set for stream!\n");
    }

protected:
    InputBase * mInputPtr;
    int64_t     mID;
    MediaType   mType;
    FrameBase * mFrame;
    int64_t     mStartTime;
    int64_t     mCurrTime;
    int64_t     mNextTime;
    int64_t     mDuration;
};

}