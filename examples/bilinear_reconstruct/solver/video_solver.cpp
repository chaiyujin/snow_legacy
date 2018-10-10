#include "video_solver.h"

#define NUM_THREADS 1

void VideoSolver::addFrame(const Landmarks &landmarks) {
    std::vector<glm::dvec2> landmarkContour;
    for (int i = 0; i < 15; ++i) { landmarkContour.push_back({ landmarks[i].x, landmarks[i].y }); }
    mContourList.push_back(landmarkContour);
    mLandmarkList.push_back(landmarks);
}

void VideoSolver::setSharedParameters(const ScaleParameter &scale, const IdenParameter &iden) {
    mScaleParam.copyFrom(&scale);
    mIdenParam.copyFrom(&iden);
    mModel.scaleParameter().copyFrom(&scale);
    mModel.idenParameter().copyFrom(&iden);
}

void VideoSolver::solve(int epochs, bool verbose) {
    const double weightContour = 1.0;
    const double weightPoint   = 1.0;
    const int    NUM_LANDMARKS = 73;
    mModel.prepareAllModel();
    mModel.updateIdenOnCore(0);
    std::vector<size_t> contourIndex(0);
    auto afterTraining = [&](void) -> void {
        mModel.exprParameter(0).useTrained();
        mModel.poseParameter(0).useTrained();
    };

    int Frames = mLandmarkList.size();
    for (int iFrame = 0; iFrame < Frames; ++iFrame) {
        std::cout << "[Frame]: " << iFrame;
        if (verbose) std::cout << std::endl; else std::cout << "\r";
        for (int iEpoch = 0; iEpoch < epochs; ++iEpoch) {
            if (verbose) std::cout << "[Epoch]: " << iEpoch << std::endl;
            // each epoch
            /* update iden */ {
                mModel.updateExpr(0);
                mModel.updateScale(0);
            }
            /* solve pose, scale */ {
                ceres::Problem problem;
                auto getSource3D = [&](int idx) -> glm::dvec3 {
                    return {
                        *mModel.mesh(0).data(idx * 3),
                        *mModel.mesh(0).data(idx * 3+1),
                        *mModel.mesh(0).data(idx * 3+2)
                    };
                };
                /* contour */
                for (size_t iContour = 0; iContour < contourIndex.size(); ++iContour) {
                    size_t idx = contourIndex[iContour];
                    auto *cost = new PoseCost2D(nullptr, 0.0, &mContourList[iFrame], weightContour,
                                                getSource3D(idx), mPVM);
                    problem.AddResidualBlock(cost, nullptr,
                                            mModel.poseParameter(0).trainRotateYXZ(),
                                            mModel.poseParameter(0).trainTranslate());
                }
                /* inner points */
                for (int iLM = 15; iLM < NUM_LANDMARKS; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    glm::dvec2 constraintP = snow::toGLM(mLandmarkList[iFrame][iLM]);
                    auto *cost = new PoseCost2D(&constraintP, weightPoint, nullptr, 0.0,
                                                getSource3D(idx), mPVM);
                    problem.AddResidualBlock(cost, nullptr,
                                            mModel.poseParameter(0).trainRotateYXZ(),
                                            mModel.poseParameter(0).trainTranslate());
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
            /* update contour */ {
                mModel.rotateYXZ(0);
                mModel.translate(0);
                contourIndex = mModel.getContourIndex(0, mPVM);
            }
            if (iEpoch < 3) continue;
            /* solve expr */ {
                double Scale = mModel.scaleParameter().param()[0];
                ceres::Problem problem;
                /* contour */
                for (size_t iContour = 0; iContour < contourIndex.size(); ++iContour) {
                    size_t idx = contourIndex[iContour];
                    auto *cost = new ExprPoseCost2D(nullptr, 0.0, &mContourList[iFrame], weightContour,
                                                    mModel.tv1e(0), idx, mPVM, Scale);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.exprParameter(0).train() + 1,
                                             mModel.poseParameter(0).trainRotateYXZ(),
                                             mModel.poseParameter(0).trainTranslate());
                }
                /* inner points */
                for (int iLM = 15; iLM < NUM_LANDMARKS; ++iLM) {
                    int idx = FaceDB::Landmarks73()[iLM];
                    glm::dvec2 constraintP = snow::toGLM(mLandmarkList[iFrame][iLM]);
                    auto *cost = new ExprPoseCost2D(&constraintP, weightPoint, nullptr, 0.0,
                                                    mModel.tv1e(0), idx, mPVM, Scale);
                    problem.AddResidualBlock(cost, nullptr,
                                             mModel.exprParameter(0).train() + 1,
                                             mModel.poseParameter(0).trainRotateYXZ(),
                                             mModel.poseParameter(0).trainTranslate());
                }
                /* reg */ {
                    auto *regExpr = new RegTerm(FaceDB::LengthExpression-1, mRegExpr);
                    problem.AddResidualBlock(regExpr, nullptr, mModel.exprParameter(0).train() + 1);  
#ifdef PARAMETER_FACS
                    for (size_t iE = 0; iE < FaceDB::LengthExpression-1; ++iE) {
                        problem.SetParameterLowerBound(mModel.exprParameter(0).train() + 1, iE,  -0.2);
                        problem.SetParameterUpperBound(mModel.exprParameter(0).train() + 1, iE,  1.2);
                    }
#endif
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
            /* update contour */ {
                mModel.updateExpr(0);
                mModel.updateScale(0);
                mModel.rotateYXZ(0);
                mModel.translate(0);
                contourIndex = mModel.getContourIndex(0, mPVM);
            }
        }

        /* use final result */
        mResultPoseList.emplace_back(mModel.poseParameter(0));
        mResultExprList.emplace_back(mModel.exprParameter(0));
    }
    std::cout << std::endl;
}