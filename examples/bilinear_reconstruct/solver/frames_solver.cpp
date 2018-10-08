#include "frames_solver.h"

#define NUM_THREADS 1

void FramesSolver::addFrame(const std::vector<snow::float2> &landmarks, const glm::dmat4 &pvm) {
    std::vector<glm::dvec2> landmarkContour;
    for (int i = 0; i < 15; ++i) { landmarkContour.push_back({ landmarks[i].x, landmarks[i].y }); }
    mContourList.push_back(landmarkContour);
    mLandmarkList.push_back(landmarks);
    mPVMList.push_back(pvm);
    mModel.appendModel();
}

void FramesSolver::solve(int epochs, bool verbose) {
    // if (epochs < 2) epochs = 2;
    const double weightContour = 0.1;
    const double weightPoint   = 1.0;
    mModel.prepareAllModel();
    std::vector<std::vector<size_t>> contourIndexList(mModel.size());
    for (int iEpoch = 0; iEpoch < epochs; ++iEpoch) {
        std::cout << "[Epoch]: " << iEpoch << std::endl;
        // each epoch
        /* update iden, expr */ for (size_t i = 0; i < mModel.size(); ++i) {
            mModel.updateIdenOnCore(i);
            mModel.updateExpr(i);
        }
        /* solve pose, scale */ {
            ceres::Problem problem;
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                for (size_t idx: contourIndexList[iMesh]) {
                    glm::dvec3 source3d = {
                        *mModel.tv11(iMesh).data(idx * 3),
                        *mModel.tv11(iMesh).data(idx * 3+1),
                        *mModel.tv11(iMesh).data(idx * 3+2)
                    };
                    auto *cost = new PoseScaleCost2D(
                        nullptr, 0.0,
                        &mContourList[iMesh], weightContour,
                        source3d, mPVMList[iMesh]);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.poseParameter(iMesh).trainRotateYXZ(),
                        mModel.poseParameter(iMesh).trainTranslate(),
                        mModel.scaleParameter().train());
                }
                for (int iLM = 15; iLM < 73; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    glm::dvec3 source3d = {
                        *mModel.tv11(iMesh).data(idx * 3),
                        *mModel.tv11(iMesh).data(idx * 3+1),
                        *mModel.tv11(iMesh).data(idx * 3+2)
                    };
                    auto constraintP = glm::dvec2 {mLandmarkList[iMesh][iLM].x, mLandmarkList[iMesh][iLM].y};
                    auto *cost = new PoseScaleCost2D(
                        &constraintP, weightPoint,
                        nullptr, 0.0,
                        source3d, mPVMList[iMesh]);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.poseParameter(iMesh).trainRotateYXZ(),
                        mModel.poseParameter(iMesh).trainTranslate(),
                        mModel.scaleParameter().train());
                }
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = verbose;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 10;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);
            mModel.idenParameter().useTrained();
            mModel.scaleParameter().useTrained();
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                mModel.exprParameter(iMesh).useTrained();
                mModel.poseParameter(iMesh).useTrained();
            }
        }
        /* update contour */ for (size_t i = 0; i < mModel.size(); ++i) {
            mModel.updateScale(i);
            mModel.rotateYXZ(i);
            mModel.translate(i);
            contourIndexList[i] = mModel.getContourIndex(i, mPVMList[i]);
        }
        /* update iden, expr */
        // if (true) continue;
        if (iEpoch < 2) continue;
        else {
            ceres::Problem problem;
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto pvmtr = mPVMList[iMesh] ;
                for (size_t idx: contourIndexList[iMesh]) {
                    auto *cost = new IdenExprScalePoseCost2D(
                        nullptr, 0.0, &mContourList[iMesh], weightContour, FaceDB::CoreTensor(), idx, pvmtr);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.idenParameter().train(),
                        mModel.exprParameter(iMesh).train(),
                        mModel.scaleParameter().train(),
                        mModel.poseParameter(iMesh).trainRotateYXZ(),
                        mModel.poseParameter(iMesh).trainTranslate());
                }
                for (int iLM = 15; iLM < 73; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    auto constraint = glm::dvec2 {mLandmarkList[iMesh][iLM].x, mLandmarkList[iMesh][iLM].y};
                    auto *cost = new IdenExprScalePoseCost2D(
                        &constraint, weightPoint, nullptr, 0.0, FaceDB::CoreTensor(), idx, pvmtr);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.idenParameter().train(),
                        mModel.exprParameter(iMesh).train(),
                        mModel.scaleParameter().train(),
                        mModel.poseParameter(iMesh).trainRotateYXZ(),
                        mModel.poseParameter(iMesh).trainTranslate());
                }
            }
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto *regExpr = new RegTerm(FaceDB::NumDimExpr(), 0.0005);
                problem.AddResidualBlock(regExpr, nullptr, mModel.exprParameter(iMesh).train());
            }
            /* reg term of iden */ {
                auto *regIden = new RegTerm(FaceDB::NumDimIden(), 0.00001);
                problem.AddResidualBlock(regIden, nullptr, mModel.idenParameter().train());
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = verbose;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 10;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);

            mModel.idenParameter().useTrained();
            mModel.scaleParameter().useTrained();
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                mModel.exprParameter(iMesh).useTrained();
                mModel.poseParameter(iMesh).useTrained();
            }
        }
        /* update contour */ for (size_t i = 0; i < mModel.size(); ++i) {
            mModel.updateIdenOnCore(i);
            mModel.updateExpr(i);
            mModel.updateScale(i);
            mModel.rotateYXZ(i);
            mModel.translate(i);
            contourIndexList[i] = mModel.getContourIndex(i, mPVMList[i]);
        }
    }

    /* update contour */ for (size_t i = 0; i < mModel.size(); ++i) {
        mModel.updateIdenOnCore(i);
        mModel.updateExpr(i);
        mModel.updateScale(i);
        mModel.rotateYXZ(i);
        mModel.translate(i);
    }
}
