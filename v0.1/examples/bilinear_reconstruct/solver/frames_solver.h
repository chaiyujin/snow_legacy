#pragma once
#include "../facedb/facedb.h"
#include "../facedb/bilinear_model.h"
#include "../visualizer/window.h"
#include "../tools/landmarks.h"
#include "cost_functions_2d.h"
#include <snow.h>

class FramesSolver {
    std::vector<Landmarks>                  mLandmarkList;
    std::vector<std::vector<glm::dvec2>>    mContourList;
    std::vector<glm::dmat4>                 mPVMList;
    BilinearModel                           mModel;
    double                                  mRegIden, mRegExpr;
public:
    FramesSolver() : mRegIden(1e-4), mRegExpr(1e-4) {}
    /* set parameters */
    void setRegIden(double reg) { mRegIden = reg; }
    void setRegExpr(double reg) { mRegExpr = reg; }
    /* add landmarks */
    void addFrame(const Landmarks &landmarks, const glm::dmat4 &pvm);
    /* solve */
    void solve(int epochs, bool verbose=true);
    /* get fitted */
    size_t                           size()              const { return mModel.size();    }
    BilinearModel &                  model()                   { return mModel;           }
    const Landmarks &                landmarks(size_t i) const { return mLandmarkList[i]; }
    const glm::dmat4 &               pvmMat(size_t i)    const { return mPVMList[i];      }
};
