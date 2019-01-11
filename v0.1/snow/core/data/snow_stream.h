/**
 * The basic streaming classes
 * 
 * An `InputBase` should generate several `StreamBase`.
 * Each `StreamBase` have the method to generate `FrameBase`.
 * 
 * The `StreamBase` is kind of output stream, it don't care about
 * how `InputBase` generate frames, it just get frame from input.
 * 
 * The data of frame is maintained by shared_ptr
 * The frame is maintained by unique_ptr
 * 
 * */
#pragma once
#include <string>
#include <memory>
#include <functional>
#include "../tools/snow_log.h"
#define SNOW_NONE_PTS       ((int64_t)UINT64_C(0x8000000000000000))
#define SNOW_FRMAE_NONE_ID  ((int64_t)UINT64_C(0x8000000000000000))

namespace snow {

enum MediaType { None=0, Audio=0x0001, Video=0x0010, AudioVideo=0x0011, Other=0x1000 };

class FrameBase;
class StreamBase;
class InputBase;

class FrameBase {
protected:
    MediaType                   mType;
    std::shared_ptr<uint8_t>    mData;
    size_t                      mDataSize;
    int64_t                     mTimestamp;
    const StreamBase *          mStreamPtr;
public:
    FrameBase(MediaType         type        = MediaType::None,
              uint8_t *         data        = nullptr,
              size_t            size        = 0,
              int64_t           ts          = SNOW_NONE_PTS,
              const StreamBase* streamPtr   = nullptr)
        : mType(type), mData(data), mDataSize(size), mTimestamp(ts), mStreamPtr(streamPtr) {}
    FrameBase(const FrameBase &b) : mType(b.mType), mData(b.mData), mDataSize(b.mDataSize), mTimestamp(b.mTimestamp), mStreamPtr(b.mStreamPtr) {}
    virtual ~FrameBase() {}

    /* get */

    MediaType           type()          const { return mType;       }
    uint8_t *           data()                { return mData.get(); }
    const uint8_t *     data()          const { return mData.get(); }
    size_t              size()          const { return mDataSize;   }
    int64_t             timestamp()     const { return mTimestamp;  }
    const StreamBase *  streamPtr()     const { return mStreamPtr;  }

    /* set */

    void setType(MediaType type)                { mType = type;    }
    void setTimestamp(int64_t ts)               { mTimestamp = ts; }
    void resetData(uint8_t *data, size_t size)  { mData.reset(data); mDataSize = size; }
    void setStreamPtr(const StreamBase *ptr)    { mStreamPtr = ptr; }

};

class InputBase {
protected:
    int64_t     mStartTime;
    int64_t     mDuration;
public:
    virtual bool open() = 0;
    virtual void seek(int64_t ms) = 0;
    virtual std::unique_ptr<FrameBase> readFrame(const StreamBase *st) = 0;
    virtual std::vector<std::shared_ptr<StreamBase>>      getStreams() = 0;

    virtual int64_t startTime() const { return mStartTime; }
    virtual int64_t duration() const { return mDuration;   }
};

class StreamBase {
protected:
    MediaType                   mType;
    int64_t                     mStartTime;
    int64_t                     mDuration;
    int64_t                     mCurrTime;
    int64_t                     mNextTime;
    InputBase *                 mInputPtr;
    std::unique_ptr<FrameBase>  mFramePtr;
    
    void checkInput() const     { if (!mInputPtr) snow::fatal("Input not set!\n"); }

public:
    StreamBase(MediaType        type       = MediaType::None,
               int64_t          start      = 0,
               int64_t          duration   = 0,
               InputBase *      inputPtr   = nullptr)
        : mType(type), mStartTime(start), mDuration(duration)
        , mCurrTime(0), mNextTime(start)
        , mInputPtr(inputPtr), mFramePtr(nullptr) {}
    virtual ~StreamBase() { }

    /* get */

    MediaType         type()        const { return mType;           }
    int64_t           startTime()   const { return mStartTime;      }
    int64_t           duration()    const { return mDuration;       }
    int64_t           currTime()    const { return mCurrTime;       }
    int64_t           nextTime()    const { return mNextTime;       }
    const InputBase * inputPtr()    const { return mInputPtr;       }
    const FrameBase * framePtr()    const { return mFramePtr.get(); }

    /* set */ 
    void setCurrTime(int64_t ms)                        { mCurrTime = ms;     }
    void setNextTime(int64_t ms)                        { mNextTime = ms;     }
    void resetFramePtr(FrameBase *frame)                { mFramePtr.reset(frame); }
    void passFramePtr(std::unique_ptr<FrameBase> frame) { mFramePtr = std::move(frame); }

    /* interface */

    virtual bool readFrame() {
        checkInput();
        mFramePtr = mInputPtr->readFrame(this);
        return mFramePtr != nullptr;
    }
    virtual void seek(int64_t ms) {
        checkInput();
        mInputPtr->seek(ms);
    }
};

}