#define SNOW_MODULE_FFMPEG
#include "process.h"
#include <sstream>

/* hyper parameters */
int SampleRate = 44100;
int AudioTrack = 0;
void setAudioTrack(int track) { AudioTrack = track; }
void setSampleRate(int sr)    { SampleRate = sr;    }

void collectVideo(const std::string &filename,
                  std::vector<ModelFrame> &modelFrames,
                  std::vector<float> &audioTrack) {
    snow::MediaReader::initializeFFmpeg();
    snow::MediaReader reader(filename);
    reader.setDstAudioSampleRate(SampleRate);
    if (!reader.open()) {
        printf("%s is not opened!\n", filename.c_str());
        return;
    }

    audioTrack.clear();
    /* get track */ {
        audioTrack = reader.audioTrack(AudioTrack);
    }

    modelFrames.clear();
    /* read bilinear parameters */ {
        std::string paramFile = filename + "_3d_params.txt";
        std::ifstream fin(paramFile);
        if (!fin.is_open()) {
            printf("%s has no bilinear parameters.\n", paramFile.c_str());
            return;
        }
        std::string str;
        int id; double val;
        while (!fin.eof()) {
            ModelFrame frame;
            fin >> frame.mTimestamp;
            if (fin.eof()) break;
            // read parameters
            fin >> str >> str >> frame.mRotation[0] >> str >> frame.mRotation[1] >> str >> frame.mRotation[2] >> str;
            fin >> str >> str >> frame.mTranslation[0] >> str >> frame.mTranslation[1] >> str >> frame.mTranslation[2] >> str;
            fin >> str >> str >> frame.mScale >> str;
            std::getline(fin, str);
            std::getline(fin, str);
            while (true) {
                std::getline(fin, str);
                snow::Trim(str);
                if (str.length() == 0) break;
                std::istringstream ssin(str);
                while (!ssin.eof()) { ssin >> val; frame.mIden.push_back(val); }
            }
            std::getline(fin, str);
            while (true) {
                std::getline(fin, str);
                snow::Trim(str);
                if (str.length() == 0) break;
                std::istringstream ssin(str);
                while (!ssin.eof()) { ssin >> id >> val; frame.mExpr.push_back(val); }
            }
            // frame.print();
            modelFrames.push_back(frame);
        }
        fin.close();
    }
    /* process identity and scale */ {
        double meanScale = 0.0;
        std::vector<double> meanIden(modelFrames[0].mIden.size(), 0.0);
        for (const auto &frame : modelFrames) {
            meanScale += frame.mScale;
            for (size_t i = 0; i < meanIden.size(); ++i) {
                meanIden[i] += frame.mIden[i];
            }
        }
        meanScale /= (double)modelFrames.size();
        for (size_t i = 0; i < meanIden.size(); ++i) {
            meanIden[i] /= (double)modelFrames.size();
        }
        // assign
        for (auto &frame : modelFrames) {
            frame.mScale = meanScale;
            for (size_t i = 0; i < meanIden.size(); ++i) {
                frame.mIden[i] = meanIden[i];
            }
        }
    }
    // close
    reader.close();
}
