#pragma once
#include "../facedb/facedb.h"
#include "../facedb/bilinear_model.h"
#include "../visualizer/window.h"
#include "cost_functions_2d.h"
#include <snow.h>

class FramesSolver {
    std::vector<std::vector<snow::float2>> mLandmarkList;
    std::vector<std::vector<glm::dvec2>>  mContourList;
    BilinearModel         mModel;
public:
    FramesSolver() {}
    void addFrame(const std::vector<snow::float2> &landmarks);
    void solve(int epochs, const glm::dmat4 &pvm);
    BilinearModel &model() { return mModel; }

    const std::vector<snow::float2> &landmarks(size_t i) const { return mLandmarkList[i]; }
};
