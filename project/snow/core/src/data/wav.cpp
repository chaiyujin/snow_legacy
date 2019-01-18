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

bool WavPcm::save(const std::string &filename, bool makeDirs) const {
    if (isNull()) {
        log::error("[WavPcm]: null!");
        return false;
    }
    if (mHeader.mSampleRate == 0) {
        log::error("[WavPcm]: sample rate is not set!");
        return false;
    }

    if (!path::exists(path::dirname(filename))) {
        log::debug("not exists {}", path::dirname(filename));
        if (!makeDirs || !path::makedirs(filename, true)) {
            log::error("[WavPcm]: save() failed to create file: {}", filename);
            return false;
        }
    }

    auto filepath = filename;
    if (!(path::extension(filepath) == ".wav")) {
        log::warn("[WavPcm]: only support .wav");
        filepath = path::change_extension(filepath, ".wav");
    }

    Header header;
    // set header
    header.mSampleRate = mHeader.mSampleRate;
    header.mChunkId[0] = 'R'; header.mChunkId[1] = 'I'; header.mChunkId[2] = 'F'; header.mChunkId[3] = 'F';
    header.mFormat[0]  = 'W'; header.mFormat[1]  = 'A'; header.mFormat[2]  = 'V'; header.mFormat[3]  = 'E';
    header.mSubChunk1Id[0] = 'f'; header.mSubChunk1Id[1] = 'm'; header.mSubChunk1Id[2] = 't'; header.mSubChunk1Id[3] = ' ';
    header.mSubChunk1Size  = 16;
    header.mAudioFormat = 1;
    header.mNumChannels = 1;  // only one track
    header.mByteRate = header.mSampleRate * 2 * 1;
    header.mBlockAlign = 16 * 1 / 8;
    header.mBitsPerSample = 16;  // int16_t
    // calc mChunkSize
    header.mChunkSize = (uint32_t)(sizeof(header) + mNumSamples * 2);

    // chunk
    Chunk chunk;
    chunk.mId[0] = 'd'; chunk.mId[1] = 'a'; chunk.mId[2] = 't'; chunk.mId[3] = 'a';
    chunk.mSize = (int32_t)mNumSamples * 2;

    std::ofstream fout(filepath, std::ios::binary);
    if (fout.is_open()) {
        fout.write((char *)(&header), sizeof(header));
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


WavPcm WavPcm::Load(const std::string &filename) {
    WavPcm wav;
    wav.load(filename);
    return wav;
}

bool WavPcm::Save(const std::string &filename, const WavPcm &wav, bool makeDirs) {
    return wav.save(filename, makeDirs);
}

}
