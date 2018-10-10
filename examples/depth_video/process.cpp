#include "process.h"
#include <sstream>

/* hyper parameters */
double FPS     = 40.0;      // 0.025s per frame
int SampleRate = 20480;     // 0.0125s per hop for 256 hop samples
int AudioTrack = 0;
void setFPS(double fps)       { FPS = fps;          }
void setAudioTrack(int track) { AudioTrack = track; }
void setSampleRate(int sr)    { SampleRate = sr;    }

double collectVideo(const std::string &filename,
                    ModelShared &modelShared,
                    std::vector<ModelFrame> &modelFrames,
                    std::vector<float> &audioTrack) {
    snow::MediaReader::initializeFFmpeg();
    snow::MediaReader reader(filename);
    reader.setDstAudioSampleRate(SampleRate);
    if (!reader.open()) {
        printf("%s is not opened!\n", filename.c_str());
        return -1.0;
    }

    /* read shared */ {
        auto pathShared = snow::path::Join(snow::path::Dirname(filename), "shared_params.txt");
        if (!snow::path::Exists(pathShared)) {
            std::cout << "Failed to find shared parameters `" << pathShared << "`\n";
            return -1.0;
        }
        std::ifstream fin(pathShared);
        fin >> modelShared;
        fin.close();
    }

    modelFrames.clear();
    /* read bilinear parameters */ {
        std::string paramFile = filename + "_pose_expr_params.txt";
        std::ifstream fin(paramFile);
        if (!fin.is_open()) {
            std::cout << "Failed to find bilinear parameters `" << paramFile << "`\n";
            return -1.0;
        }
        int num; fin >> num;
        while (num--) {
            ModelFrame frame;
            fin >> frame;
            modelFrames.push_back(frame);
        }
        fin.close();
    }

    double fps = reader.fps();
    /* resample */ if (abs(fps - FPS) > 1e-6) {
        printf("[resample]: %.2f -> %.2f\n", fps, FPS);
        // copy first
        std::vector<ModelFrame> resampled;
        resampled.push_back(modelFrames[0]);
        double deltaMs = 1000.0 / FPS;
        size_t srcI = 0;
        for (double ms = resampled[0].mTimestamp; srcI + 1 < modelFrames.size() ; ms += deltaMs) {
            ms = std::round(ms);
            while (srcI + 1 < modelFrames.size() && modelFrames[srcI + 1].mTimestamp < ms) ++srcI;
            if (srcI + 1 >= modelFrames.size()) break;
            resampled.emplace_back();
            resampled.back().mTimestamp = (int64_t)ms;
            double l = (double)modelFrames[srcI].mTimestamp;
            double r = (double)modelFrames[srcI + 1].mTimestamp;
            double a = (r - ms) / (r - l);
            for (size_t i = 0; i < modelFrames[0].mExpr.size(); ++i) {
                resampled.back().mExpr.push_back(a         * modelFrames[srcI].mExpr[i] + 
                                                 (1.0 - a) * modelFrames[srcI + 1].mExpr[i]);
            }
        }
        modelFrames = resampled;
        fps = FPS;
    }

    /* get track */ {
        audioTrack.clear();
        int64_t vst = modelFrames[0].mTimestamp;
        int64_t ast = reader.audioTrackStartTime(AudioTrack);
        size_t startPos = 0;
        if (ast > vst) { // padding
            int padding = (ast - vst) * SampleRate / 1000.0;
            for (int i = 0; i < padding; ++i)
                audioTrack.push_back(0.0f);
            printf("[audio]: pad %d samples\n", padding);
        }
        else if (ast < vst) { // dropping
            int dropping = (vst - ast) * SampleRate / 1000.0;
            startPos += (size_t)dropping;
            printf("[audio]: drop %d samples\n", dropping);
        }
        const auto &track = reader.audioTrack(AudioTrack);
        for (size_t i = startPos; i < track.size(); ++i)
            audioTrack.push_back(track[i]);
    }

    // close
    reader.close();
    return fps;
}
