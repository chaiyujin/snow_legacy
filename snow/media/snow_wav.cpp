#include "snow_wav.h"
#include <iostream>

namespace snow {

bool WavPCM::read(const std::string &path) {
    if (!std::ifstream(path).good()) {
        printf("[WavPCM] No such file: %s\n", path.c_str());
        return false;
    }
    
    std::ifstream fin(path, std::ios::binary);

    fin.read((char *)(&mHeader), sizeof(mHeader));

    /* check if the mHeader is valid */ {    
        if (mHeader.mSubChunk1Size < 16) {
            printf("[WavPCM]: Wrong mHeader %s.\n", path.c_str());
            return false;
        }
        if (mHeader.mAudioFormat != 1) {
            printf("[WavPCM]: Not PCM %s\n", path.c_str());
            return false;
        }
    }

    // skip extra infomations
    if (mHeader.mSubChunk1Size > 16)
        fin.seekg(mHeader.mSubChunk1Size - 16, std::ios_base::cur); 

    Chunk chunk;
    for (;;) {
        fin.read((char *)&(chunk), sizeof(Chunk));
        // printf("%c%c%c%c\t%d\n", chunk.id[0], chunk.id[1], chunk.id[2], chunk.id[3], chunk.size);
        if (*(uint32_t *)&chunk.mId == 0x61746164)  // == "data"
            break;
        fin.seekg(chunk.mSize, std::ios_base::cur);  // skip not important subchunk
    }

    int samples_count = chunk.mSize * 8 / mHeader.mBitsPerSample / mHeader.mNumChannels;

    // printf("Samples count = %i\n", samples_count);
    mData.clear();
    for (uint16_t i = 0; i < mHeader.mNumChannels; ++i) {
        mData.emplace_back(samples_count);
    }

    for (int i = 0; i < samples_count; ++i) {
        for (int ch = 0; ch < mHeader.mNumChannels; ++ch) {
            if (fin.eof()) {
                printf("[WavPCM] Reach end of file.\n");
                return false;
            }
            // write data according to bits
            switch (mHeader.mBitsPerSample) {
            case 8: {
                uint8_t val;
                fin.read((char *)&val, 1);
                mData[ch][i] = (((float)val - 128.0f) / 128.0f);
                break;
            }
            case 16: {
                int16_t val;
                fin.read((char *)&val, 2);
                mData[ch][i] = ((float)val / 32767.0f);
                break;
            }
            default:
                printf("[WavPCM] unsupport bit_depth %d\n", mHeader.mBitsPerSample);
                return false;
            }
        }
    }

    // printf("%lu channels\n", data_.size());
    // for (size_t i = 0; i < data_.size(); ++i)
        // printf("%lu samples\n", data_[i].size());

    fin.close();

    return true;
}

bool WavPCM::write(const std::string &path) {
    if (mData.size() == 0)        { printf("No tracks!\n"); return false;               }
    if (mData.size() > 1)         { printf("Ignore channels rather than channel-0.\n"); }
    if (mHeader.mSampleRate == 0) { printf("Sample rate is not set!\n"); return false;  }
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
    mHeader.mChunkSize = (uint32_t)(sizeof(mHeader) + channel(0).size() * 2);

    // chunk
    Chunk chunk;
    chunk.mId[0] = 'd'; chunk.mId[1] = 'a'; chunk.mId[2] = 't'; chunk.mId[3] = 'a';
    chunk.mSize = (int32_t)channel(0).size() * 2;

    std::ofstream fout(path, std::ios::binary);
    if (fout.is_open()) {
        fout.write((char *)(&mHeader), sizeof(mHeader));
        fout.write((char *)(&chunk),   sizeof(chunk));
        for (float x : channel(0)) {
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

void WavPCM::dumpHeader() const {
    printf("WAV PCM File mHeader: \n");
    printf("File Type: %c%c%c%c\n", mHeader.mChunkId[0], mHeader.mChunkId[1], mHeader.mChunkId[2], mHeader.mChunkId[3]);
    printf("File Size: %d\n", mHeader.mChunkSize);
    printf("Format Name: %c%c%c%c\n", mHeader.mFormat[0], mHeader.mFormat[1], mHeader.mFormat[2], mHeader.mFormat[3]);
    printf("Format chunk: %c%c%c%c\n", mHeader.mSubChunk1Id[0], mHeader.mSubChunk1Id[1], mHeader.mSubChunk1Id[2], mHeader.mSubChunk1Id[3]);
    printf("Format Length: %d\n", mHeader.mSubChunk1Size);
    printf("Format Type: %d\n", mHeader.mAudioFormat);
    printf("Number of channels: %d\n", mHeader.mNumChannels);
    printf("Sample Rate: %d\n", mHeader.mSampleRate);
    printf("Sample Rate * Bits/Sample * Channels / 8: %d\n", mHeader.mByteRate);
    printf("Bits per Sample * Channels / 8: %d\n", mHeader.mBlockAlign);
    printf("Bits per Sample: %d\n", mHeader.mBitsPerSample);
}


}