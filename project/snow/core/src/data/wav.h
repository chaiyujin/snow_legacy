// #pragma once
// #include "../common.h"
// #include "../memory/allocator.h"

// namespace snow {

// /**
//  * Mono channel wav in pcm format
//  * */
// class WavPcm {
// public:
//     struct Header {
//         char            mChunkId[4];
//         uint32_t        mChunkSize;
//         char            mFormat[4];
//         char            mSubChunk1Id[4];
//         uint32_t        mSubChunk1Size;
//         uint16_t        mAudioFormat;
//         uint16_t        mNumChannels;
//         uint32_t        mSampleRate;
//         uint32_t        mByteRate;
//         uint16_t        mBlockAlign;
//         uint16_t        mBitsPerSample;
//     };

//     struct Chunk {
//         char            mId[4];
//         int32_t         mSize;
//     };

// private:
//     using floats = std::shared_ptr<float>;
//     Header   mHeader;
//     floats   mData;
//     uint32_t mNumSamples;
//     int64_t  mStartTime;

// public:
//     WavPcm() : mData(nullptr, memory::deallocate)
//              , mNumSamples(0)
//              , mStartTime(0) { mHeader.mSampleRate = 0; }
//     WavPcm(uint32_t sampleRate) : mStartTime(0) { mHeader.mSampleRate = sampleRate; }

//     bool load(const std::string &path);
//     bool save(const std::string &path);
//     /* set */
//     void setData(const float *dataPtr, uint32_t numSamples);
//     void setSampleRate(uint32_t sr) { mHeader.mSampleRate = sr; }
//     /* get */
//     size_t numChannels() const { return mData.size(); }
//     uint32_t sampleRate() const { return mHeader.mSampleRate; }
//     int64_t  startTime() const { return mHeader.mStartTime; }
//     /* debug */
//     void dumpHeader() const;
// };

// }