#pragma once
#include "../common.h"
#include "../memory/allocator.h"

namespace snow {

/**
 * Mono channel wav in pcm format
 * */
class SNOW_API WavPcm {
public:
    struct Header {
        char            mChunkId[4];
        uint32_t        mChunkSize;
        char            mFormat[4];
        char            mSubChunk1Id[4];
        uint32_t        mSubChunk1Size;
        uint16_t        mAudioFormat;
        uint16_t        mNumChannels;
        uint32_t        mSampleRate;
        uint32_t        mByteRate;
        uint16_t        mBlockAlign;
        uint16_t        mBitsPerSample;
    };

    struct Chunk {
        char            mId[4];
        int32_t         mSize;
    };

private:
    using floats = std::shared_ptr<float>;
    Header   mHeader;
    floats   mData;
    uint32_t mNumSamples;
    int64_t  mStartTime;  // record start-time shift

public:
    WavPcm() : mData(nullptr, memory::deallocate)
             , mNumSamples(0)
             , mStartTime(0) { mHeader.mSampleRate = 0; }
    WavPcm(uint32_t sampleRate) : WavPcm() { mHeader.mSampleRate = sampleRate; }

    bool load(const std::string &path);
    bool save(const std::string &path, bool makeDirs=false) const;
    bool isNull() const { return mData == nullptr; }
    /* set */
    void setData(const float *dataPtr, uint32_t numSamples);
    void setSampleRate(uint32_t sr) { mHeader.mSampleRate = sr; }
    /* get */
    SNOW_FORCE_INLINE float *data() { return mData.get(); }
    SNOW_FORCE_INLINE const float *data() const { return mData.get(); }
    SNOW_FORCE_INLINE size_t numSampels() const { return mNumSamples; }
    SNOW_FORCE_INLINE size_t numChannels() const { return 1; }
    SNOW_FORCE_INLINE uint32_t sampleRate() const { return mHeader.mSampleRate; }
    SNOW_FORCE_INLINE int64_t  startTime() const { return mStartTime; }
    std::vector<float> channel() const;
    /* debug */
    void dumpHeader() const;

    /* static methods */
    static WavPcm Load(const std::string &filename);
    static bool Save(const std::string &filename, const WavPcm &wav, bool makeDirs=false);
};

}
