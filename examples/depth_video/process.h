#pragma once
#include <snow.h>
#include <string>

#include "data.h"

extern int SampleRate;

void   setFPS(double fps);
void   setAudioTrack(int track);
void   setSampleRate(int sr); 
double collectVideo(const std::string &filename,
                    ModelShared &modelShared,
                    std::vector<ModelFrame> &modelFrames,
                    std::vector<float> &audioTrack);
