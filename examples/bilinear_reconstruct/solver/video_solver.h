#pragma once
#include "../facedb/facedb.h"
#include "../facedb/bilinear_model.h"
#include "../visualizer/window.h"
#include "../tools/landmarks.h"
#include "cost_functions_2d.h"
#include <snow.h>

class VideoSolver {
    ScaleParameter                          mScaleParam;
    IdenParameter                           mIdenParam;
    std::vector<Landmarks>                  mLandmarkList;
    std::vector<std::vector<glm::dvec2>>    mContourList;
    glm::dmat4                              mPVM;
    BilinearModel                           mModel;
    double                                  mRegExpr;

    std::vector<PoseParameter>              mResultPoseList;
    std::vector<ExprParameter>              mResultExprList;
public:
    VideoSolver() : mPVM(1.0), mRegExpr(1e-4) { mModel.appendModel();   }
    /* set parameters */
    void setRegExpr(double reg)               { mRegExpr = reg;         }
    void setPVMMat(const glm::dmat4 &pvm)     { mPVM     = pvm;         }
    void setSharedParameters(const ScaleParameter &scale, const IdenParameter &iden);
    /* add landmarks */
    void addFrame(const Landmarks &landmarks);
    /* solve */
    void solve(int epochs, bool verbose=true);
    /* get fitted */
    size_t                           numResults()        const { return mResultPoseList.size(); }
    size_t                           size()              const { return mModel.size();    }
    BilinearModel &                  model()                   { return mModel;           }
    const Landmarks &                landmarks(size_t i) const { return mLandmarkList[i]; }
    const glm::dmat4 &               pvmMat()            const { return mPVM;             }
    const std::vector<Landmarks> &   landmarksList()     const { return mLandmarkList;    }

    const std::vector<PoseParameter>&resultPoseList()    const { return mResultPoseList;  }
    const std::vector<ExprParameter>&resultExprList()    const { return mResultExprList;  }
};
