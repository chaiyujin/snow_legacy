#include "frames_solver.h"

#define NUM_THREADS 4

void FramesSolver::addFrame(const Landmarks &landmarks, const glm::dmat4 &pvm) {
    std::vector<glm::dvec2> landmarkContour;
    for (int i = 0; i < 15; ++i) { landmarkContour.push_back({ landmarks[i].x, landmarks[i].y }); }
    mContourList.push_back(landmarkContour);
    mLandmarkList.push_back(landmarks);
    mPVMList.push_back(pvm);
    mModel.appendModel();
}

void FramesSolver::solve(int epochs, bool verbose) {
    const double WeightContour = 0.5;
    const double WeightPoint   = 1.0;
    const int    NUM_LANDMARKS = Landmarks::Numbers;
    mModel.prepareAllModel();
    std::vector<std::vector<size_t>> contourIndexList(mModel.size());
    auto afterTraining = [&](void) -> void {
        mModel.idenParameter().useTrained();
        mModel.scaleParameter().useTrained();
        for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
            mModel.exprParameter(iMesh).useTrained();
            mModel.poseParameter(iMesh).useTrained();
        }
    };
    for (int iEpoch = 0; iEpoch < epochs; ++iEpoch) {
        std::cout << "[Epoch]: " << iEpoch << std::endl;
        // each epoch
        /* update iden, expr */ for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
            mModel.updateIdenOnCore(iMesh);
            mModel.updateExpr(iMesh);
        }
        /* solve pose, scale */ {
            ceres::Problem problem;
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto getSource3D = [&](int idx) -> glm::dvec3 {
                    return {
                        *mModel.tv11(iMesh).data(idx * 3),
                        *mModel.tv11(iMesh).data(idx * 3+1),
                        *mModel.tv11(iMesh).data(idx * 3+2)
                    };
                };
                /* contour */
                for (size_t iContour = 0; iContour < contourIndexList[iMesh].size(); ++iContour) {
                    double w = 8.0;
                    size_t idx = contourIndexList[iMesh][iContour];
                    auto *cost = new PoseScaleCost2D(nullptr, 0.0, &mContourList[iMesh], WeightContour * w,
                                                     getSource3D(idx), mPVMList[iMesh]);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.poseParameter(iMesh).trainRotateYXZ(),
                                             mModel.poseParameter(iMesh).trainTranslate(),
                                             mModel.scaleParameter().train());
                }
                /* inner points */
                for (int iLM = 15; iLM < NUM_LANDMARKS; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    glm::dvec2 constraintP = snow::toGLM(mLandmarkList[iMesh][iLM]);
                    auto *cost = new PoseScaleCost2D(&constraintP, WeightPoint, nullptr, 0.0,
                                                     getSource3D(idx), mPVMList[iMesh]);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.poseParameter(iMesh).trainRotateYXZ(),
                                             mModel.poseParameter(iMesh).trainTranslate(),
                                             mModel.scaleParameter().train());
                }
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = verbose;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 50;
            options.initial_trust_region_radius = 0.1;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);

            afterTraining();
        }
        /* update contour */ for (size_t i = 0; i < mModel.size(); ++i) {
            mModel.updateScale(i);
            mModel.rotateYXZ(i);
            mModel.translate(i);
            contourIndexList[i] = mModel.getContourIndex(i, mPVMList[i]);
        }
        /* update iden, expr */
        if (iEpoch < 4) continue;
        else {
            ceres::Problem problem;
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
                auto pvm = mPVMList[iMesh];
                for (size_t idx: contourIndexList[iMesh]) {
                    auto *cost = new IdenExprScalePoseCost2D(nullptr, 0.0, &mContourList[iMesh], WeightContour,
                                                             FaceDB::CoreTensor(), (int)idx, pvm);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.idenParameter().train()      + 1,
                                             mModel.exprParameter(iMesh).train() + 1,
                                             mModel.scaleParameter().train(),
                                             mModel.poseParameter(iMesh).trainRotateYXZ(),
                                             mModel.poseParameter(iMesh).trainTranslate());
                }
                for (int iLM = 15; iLM < NUM_LANDMARKS; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    auto constraint = glm::dvec2 {mLandmarkList[iMesh][iLM].x, mLandmarkList[iMesh][iLM].y};
                    auto *cost = new IdenExprScalePoseCost2D(&constraint, WeightPoint, nullptr, 0.0,
                                                             FaceDB::CoreTensor(), (int)idx, pvm);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.idenParameter().train()      + 1,
                                             mModel.exprParameter(iMesh).train() + 1,
                                             mModel.scaleParameter().train(),
                                             mModel.poseParameter(iMesh).trainRotateYXZ(),
                                             mModel.poseParameter(iMesh).trainTranslate());
                }
            }
            // for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {
            //     auto pvm = mPVMList[iMesh] ;
            //     auto tr  = mModel.poseParameter(iMesh).matT() * mModel.poseParameter(iMesh).matR(); 
            //     for (size_t idx: contourIndexList[iMesh]) {
            //         auto *cost = new IdenExprScaleCost2D(nullptr, 0.0, &mContourList[iMesh], WeightContour,
            //                                              FaceDB::CoreTensor(), (int)idx, pvm * tr);
            //         problem.AddResidualBlock(cost, nullptr,
            //                                  mModel.idenParameter().train() + 1,
            //                                  mModel.exprParameter(iMesh).train() + 1,
            //                                  mModel.scaleParameter().train());
            //     }
            //     for (int iLM = 15; iLM < NUM_LANDMARKS; ++iLM) {
            //         int idx = FaceDB::Landmarks73()[iLM];
            //         auto constraint = glm::dvec2 {mLandmarkList[iMesh][iLM].x, mLandmarkList[iMesh][iLM].y};
            //         auto *cost = new IdenExprScaleCost2D(&constraint, WeightPoint, nullptr, 0.0,
            //                                              FaceDB::CoreTensor(), (int)idx, pvm * tr);
            //         problem.AddResidualBlock(cost, nullptr,
            //                                  mModel.idenParameter().train() + 1,
            //                                  mModel.exprParameter(iMesh).train() + 1,
            //                                  mModel.scaleParameter().train());
            //     }
            // }
            /* reg term of expr */
            for (size_t iMesh = 0; iMesh < mModel.size(); ++iMesh) {     
                auto *regExpr = new RegTerm(FaceDB::LengthExpression-1, mRegExpr);
                problem.AddResidualBlock(regExpr, nullptr, mModel.exprParameter(iMesh).train() + 1);  
#ifdef PARAMETER_FACS
                for (size_t iE = 0; iE < FaceDB::LengthExpression-1; ++iE) {
                    problem.SetParameterLowerBound(mModel.exprParameter(iMesh).train() + 1, iE,  -0.2);
                    problem.SetParameterUpperBound(mModel.exprParameter(iMesh).train() + 1, iE,  1.2);
                }
#endif
            }
            /* reg term of iden */ {
                auto *regIden = new RegTerm(FaceDB::LengthIdentity-1, mRegIden);
                problem.AddResidualBlock(regIden, nullptr, mModel.idenParameter().train() + 1);
            }
            ceres::Solver::Options options;
            options.minimizer_progress_to_stdout = verbose;
            options.num_threads = NUM_THREADS;
            options.max_num_iterations = 50;
            options.initial_trust_region_radius = 0.1;
            ceres::Solver::Summary summary;
            ceres::Solve(options, &problem, &summary);
            
            afterTraining();
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
    /* after training */ for (size_t i = 0; i < mModel.size(); ++i) {
        mModel.updateIdenOnCore(i);
        mModel.updateExpr(i);
        mModel.updateScale(i);
        mModel.rotateYXZ(i);
        mModel.translate(i);
        contourIndexList[i] = mModel.getContourIndex(i, mPVMList[i]);
    }
}
