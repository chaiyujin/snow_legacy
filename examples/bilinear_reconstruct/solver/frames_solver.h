#pragma once
#include "../facedb/facedb.h"
#include "../facedb/bilinear_model.h"
#include "../visualizer/window.h"
#include "cost_functions_2d.h"
#include <snow.h>

typedef std::vector<snow::float2> Landmark;
typedef std::vector<glm::dvec2> Contour;
class FramesSolver {
    std::vector<Landmark> mLandmarkList;
    std::vector<Contour>  mContourList;
    BilinearModel         mModel;
public:
    FramesSolver() {}
    void addFrame(const Landmark &landmarks);
    void solve(int epochs, const glm::dmat4 &pvm);
    BilinearModel &model() { return mModel; }
};
