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
                mData[ch][i] = (((double)val - 128.0) / 128.0);
                break;
            }
            case 16: {
                int16_t val;
                fin.read((char *)&val, 2);
                mData[ch][i] = ((double)val / 32768.);
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
    return true;
}    

}