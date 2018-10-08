#pragma once
#include "../facedb/facedb.h"
#include "../facedb/bilinear_model.h"
#include "../visualizer/window.h"
#include "cost_functions_2d.h"
#include <snow.h>

class FramesSolver {
    std::vector<std::vector<snow::float2>>  mLandmarkList;
    std::vector<std::vector<glm::dvec2>>    mContourList;
    std::vector<glm::dmat4>                 mPVMList;
    BilinearModel                           mModel;
public:
    FramesSolver() {}
    void addFrame(const std::vector<snow::float2> &landmarks, const glm::dmat4 &pvm);
    void solve(int epochs, bool verbose=true);
    BilinearModel &model() { return mModel; }

    const std::vector<snow::float2> &landmarks(size_t i) const { return mLandmarkList[i]; }
};
