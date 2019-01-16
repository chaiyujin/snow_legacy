#include "wav.h"
#include "../tools/log.h"
#include "../tools/path.h"
#include <fstream>

namespace snow {

bool WavPcm::load(const std::string &path) {
    if (!path::exists(path)) {
        log::error("[WavPcm]: No such file `{0}`", path);
        return false;
    }
    
    std::ifstream fin(path, std::ios::binary);

    fin.read((char *)(&mHeader), sizeof(mHeader));

    /* check if the mHeader is valid */ {    
        if (mHeader.mSubChunk1Size < 16) {
            log::error("[WavPcm]: Wrong mHeader `{0}`", path);
            return false;
        }
        if (mHeader.mAudioFormat != 1) {
            log::error("[WavPcm]: Not PCM `{0}`", path);
            return false;
        }
    }

    // skip extra infomations
    if (mHeader.mSubChunk1Size > 16)
        fin.seekg(mHeader.mSubChunk1Size - 16, std::ios_base::cur); 

    Chunk chunk;
    for (;;) {
        fin.read((char *)&(chunk), sizeof(Chunk));
        if (chunk.mId[0] == 'd' && chunk.mId[1] == 'a' && chunk.mId[2] == 't' && chunk.mId[3] == 'a')
            break;
        fin.seekg(chunk.mSize, std::ios_base::cur);  // skip not important subchunk
    }

    mNumSamples = chunk.mSize * 8 / mHeader.mBitsPerSample / mHeader.mNumChannels;
    if (mHeader.mNumChannels > 1) {
        log::warn("[WavPcm]: only support mono channels, ignore other channels.");
    }
    mData.reset(memory::allocate<float>(mNumSamples), memory::deallocate);
    
    for (uint32_t i = 0; i < mNumSamples; ++i) {
        for (int ch = 0; ch < mHeader.mNumChannels; ++ch) {
            if (fin.eof()) {
                log::error("[WavPcm] Reach end of file.");
                return false;
            }
            // write data according to bits
            switch (mHeader.mBitsPerSample) {
            case 8: {
                uint8_t val;
                fin.read((char *)&val, 1);
                if (ch == 0) data()[i] = (((float)val - 128.0f) / 128.0f);
                break;
            }
            case 16: {
                int16_t val;
                fin.read((char *)&val, 2);
                if (ch == 0) data()[i] = ((float)val / 32767.0f);
                break;
            }
            default:
                log::error("[WavPcm] unsupport bit_depth {0:d}", mHeader.mBitsPerSample);
                return false;
            }
        }
    }

    fin.close();

    return true;
}

bool WavPcm::save(const std::string &path) {
    if (isNull()) { log::error("[WavPcm]: null!"); return false;               }
    if (mHeader.mSampleRate == 0) { log::error("[WavPcm]: sample rate is not set!"); return false; }
    // set header
    mHeader.mChunkId[0] = 'R'; mHeader.mChunkId[1] = 'I'; mHeader.mChunkId[2] = 'F'; mHeader.mChunkId[3] = 'F';
    mHeader.mFormat[0]  = 'W'; mHeader.mFormat[1]  = 'A'; mHeader.mFormat[2]  = 'V'; mHeader.mFormat[3]  = 'E';
    mHeader.mSubChunk1Id[0] = 'f'; mHeader.mSubChunk1Id[1] = 'm'; mHeader.mSubChunk1Id[2] = 't'; mHeader.mSubChunk1Id[3] = ' ';
    mHeader.mSubChunk1Size  = 16;
    mHeader.mAudioFormat = 1;
    mHeader.mNumChannels = 1;  // only one track
    mHeader.mByteRate = mHeader.mSampleRate * 2 * 1;
    mHeader.mBlockAlign = 16 * 1 / 8;
    mHeader.mBitsPerSample = 16;  // int16_t
    // calc mChunkSize
    mHeader.mChunkSize = (uint32_t)(sizeof(mHeader) + mNumSamples * 2);

    // chunk
    Chunk chunk;
    chunk.mId[0] = 'd'; chunk.mId[1] = 'a'; chunk.mId[2] = 't'; chunk.mId[3] = 'a';
    chunk.mSize = (int32_t)mNumSamples * 2;

    std::ofstream fout(path, std::ios::binary);
    if (fout.is_open()) {
        fout.write((char *)(&mHeader), sizeof(mHeader));
        fout.write((char *)(&chunk),   sizeof(chunk));
        for (uint32_t i = 0; i < mNumSamples; ++i) {
            float x = data()[i];
            if (x > 1.0)  x = 1.0;
            if (x < -1.0) x = -1.0;
            int16_t sample = (int16_t)(x * 32767.0f);
            fout.write((char *)(&sample), sizeof(int16_t));
        }
        fout.close();
        return true;
    }
    else {
        return false;
    }
}

void WavPcm::setData(const float *dataPtr, uint32_t numSamples) {
    mNumSamples = numSamples;
    mData.reset(memory::allocate<float>(numSamples), memory::deallocate);
    for (uint32_t i = 0; i < numSamples; ++i)
        data()[i] = dataPtr[i];
}

std::vector<float> WavPcm::channel() const {
    std::vector<float> ret(mNumSamples);
    memcpy(ret.data(), mData.get(), sizeof(float) * mNumSamples);
    return ret;
}

void WavPcm::dumpHeader() const {
    log::info(
        "WAV PCM file header\n"
        "  File Type:     {}{}{}{}, File Size:    {}\n"
        "  Format Name:   {}{}{}{}, Format Chunk: {}{}{}{}\n"
        "  Format Length: {:4d}, Format Type:  {:d}\n"
        "  Channels:      {:4d}, Sample Rate:  {:d}\n"
        "  BitsPerSample: {:4d}",
        (char)mHeader.mChunkId[0], (char)mHeader.mChunkId[1],
        (char)mHeader.mChunkId[2], (char)mHeader.mChunkId[3],
        mHeader.mChunkSize,
        (char)mHeader.mFormat[0], (char)mHeader.mFormat[1],
        (char)mHeader.mFormat[2], (char)mHeader.mFormat[3],
        (char)mHeader.mSubChunk1Id[0], (char)mHeader.mSubChunk1Id[1],
        (char)mHeader.mSubChunk1Id[2], (char)mHeader.mSubChunk1Id[3],
        mHeader.mSubChunk1Size,
        mHeader.mAudioFormat,
        mHeader.mNumChannels,
        mHeader.mSampleRate,
        mHeader.mBitsPerSample
    );
}

}
