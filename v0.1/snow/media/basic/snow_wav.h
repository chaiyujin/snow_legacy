/* Wave File Format
Reference : http://ccrma.stanford.edu/CCRMA/Courses/422/projects/WaveFormat/
The canonical WAVE format starts with the RIFF header:
0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk 
                               following this number.  This is the size of the 
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).
The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:
12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some 
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters
The "data" subchunk contains the size of the data and the actual sound:
36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this 
                               number.
44        *   Data             The actual sound data.
*/

#pragma once
#include <string>
#include <fstream>
#include <vector>


namespace snow {

class WavPCM {
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

    WavPCM() : mStartTime(0) { mHeader.mSampleRate = 0; }

    bool read (const std::string &path);
    bool write(const std::string &path);
    void setSampleRate(uint32_t sampleRate)             { mHeader.mSampleRate = sampleRate; }
    void addChannel(const std::vector<float> &channel)  { mData.push_back(channel);         }
    void setStartTime(int64_t st)                       { mStartTime = st;                   }

    std::vector<float> &        channel(int i)          { return mData[i];                  }
    const std::vector<float> &  channel(int i)    const { return mData[i];                  }
    size_t                      numChannels()     const { return mData.size();              }
    uint32_t                    sampleRate()      const { return mHeader.mSampleRate;       }
    int64_t                     startTime()       const { return mStartTime;                }
    void                        dumpHeader()      const;

private:
    std::vector<std::vector<float>>   mData;
    Header                            mHeader;
    int64_t                           mStartTime;
};

}