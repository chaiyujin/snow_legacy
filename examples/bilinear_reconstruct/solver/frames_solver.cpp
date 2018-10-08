#include "frames_solver.h"

#define NUM_THREADS 4

void FramesSolver::addFrame(const std::vector<snow::float2> &landmarks, const glm::dmat4 &pvm) {
    std::vector<glm::dvec2> landmarkContour;
    for (int i = 0; i < 15; ++i) { landmarkContour.push_back({ landmarks[i].x, landmarks[i].y }); }
    mContourList.push_back(landmarkContour);
    mLandmarkList.push_back(landmarks);
    mPVMList.push_back(pvm);
    mModel.appendModel();
}

void FramesSolver::solve(int epochs) {
    if (epochs < 2) epochs = 2;
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
                        &mContourList[iMesh], 1.0,
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
                        &constraintP, 1.0,
                        nullptr, 0.0,
                        source3d, mPVMList[iMesh]);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.poseParameter(iMesh).trainRotateYXZ(),
                        mModel.poseParameter(iMesh).trainTranslate(),
                        mModel.scaleParameter().train());
                }
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = true;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 10;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                mModel.poseParameter(iMesh).useTrained();
            }
            mModel.scaleParameter().useTrained();
        }
        /* update contour */ for (size_t i = 0; i < mModel.size(); ++i) {
            mModel.updateScale(i);
            mModel.rotateYXZ(i);
            mModel.translate(i);
            contourIndexList[i] = mModel.getContourIndex(i, mPVMList[i]);
        }
        /* update iden, expr */
        // if (true) continue;
        if (iEpoch < 1) continue;
        else {
            ceres::Problem problem;
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto pvmtr = mPVMList[iMesh] * mModel.poseParameter(iMesh).matT() * mModel.poseParameter(iMesh).matR();
                for (size_t idx: contourIndexList[iMesh]) {
                    auto *cost = new IdenExprScaleCostCost2D(
                        nullptr, 0.0, &mContourList[iMesh], 1.0, FaceDB::CoreTensor(), idx, pvmtr);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.idenParameter().train(),
                        mModel.exprParameter(iMesh).train(),
                        mModel.scaleParameter().train());
                }
                for (int iLM = 15; iLM < 73; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    auto constraint = glm::dvec2 {mLandmarkList[iMesh][iLM].x, mLandmarkList[iMesh][iLM].y};
                    auto *cost = new IdenExprScaleCostCost2D(
                        &constraint, 1.0, nullptr, 0.0, FaceDB::CoreTensor(), idx, pvmtr);
                    problem.AddResidualBlock(cost, nullptr,
                        mModel.idenParameter().train(),
                        mModel.exprParameter(iMesh).train(),
                        mModel.scaleParameter().train());
                }
            }
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto *regExpr = new RegTerm(FaceDB::NumDimExpr(), 0.0005);
                problem.AddResidualBlock(regExpr, nullptr, mModel.exprParameter(iMesh).train());
            }
            /* reg term of iden */ {
                auto *regIden = new RegTerm(FaceDB::NumDimIden(), 0.0001);
                problem.AddResidualBlock(regIden, nullptr, mModel.idenParameter().train());
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = true;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 10;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);

            mModel.idenParameter().useTrained();
            mModel.scaleParameter().useTrained();
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                mModel.exprParameter(iMesh).useTrained();
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
