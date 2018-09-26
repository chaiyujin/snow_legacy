#pragma once
#include <snow.h>
#include <string>

#include "data.h"

void setAudioTrack(int track);
void setSampleRate(int sr); 
void collectVideo(const std::string &filename,
                  std::vector<ModelFrame> &modelFrames,
                  std::vector<float> &audioTrack);
