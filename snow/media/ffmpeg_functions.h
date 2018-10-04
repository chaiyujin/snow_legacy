#pragma once
#include "ffmpeg_head.h"
#include <vector>

namespace snow {
namespace ffmpeg {

int decode(AVCodecContext *avctx, AVFrame  *frame, int *gotFrame, AVPacket *pkt);
int encode(AVCodecContext *avctx, AVPacket *pkt,   int *gotPacket, AVFrame *frame);
AVFrame *allocAudioFrame(enum AVSampleFormat sampleFmt, uint64_t channelLayout, int sampleRate, int nbSamples);
AVFrame *allocPicture(enum AVPixelFormat pixFmt, int width, int height);
std::vector<float> resample(const std::vector<float> &audio, int srcSampleRate, int dstSampleRate);

}
}