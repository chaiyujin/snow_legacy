#pragma once
#include <ceres/ceres.h>
#include <snow.h>
#include "../tools/math_tools.h"
#include "modules.h"

template <typename T>
struct _Constraint {
    T      data;
    double weight;
};
typedef _Constraint<glm::dvec2> Constraint2D;
typedef _Constraint<glm::dvec3> Constraint3D;


struct PoseScaleCost2DPoint : public ceres::SizedCostFunction<1, 3, 3, 1> {
    CriterionPointToPoint2D mCriterion;
    glm::dvec4              mSource;
    glm::dmat4              mPVM;
    
    PoseScaleCost2DPoint(const Constraint2D &constraint,
                    const glm::dvec3   &source3d,
                    const glm::dmat4   &pvm)
        : mCriterion(constraint.data, constraint.weight)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {}

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        // from parameter
        RotateYXZ rotateYXZ(params[0][0], params[0][1], params[0][2]);
        Translate translate(params[1][0], params[1][1], params[1][2]);
        Scale     scale(params[2][0]);
        Project   project(mPVM);
        Sequential seq;
        seq.addModule(&scale);
        seq.addModule(&rotateYXZ);
        seq.addModule(&translate);
        seq.addModule(&project);
        auto Q       = seq.forward(mSource);
        auto point   = glm::dvec2{Q.x, Q.y};
        residuals[0] = mCriterion.forward(point);
        auto grad2   = mCriterion.backward(point);
        auto gradIn  = glm::dvec4 {grad2.x, grad2.y, 0., 0.};
        seq.backward(gradIn);
        if (jacobians && jacobians[0]) { // jacobians for rotation
            std::vector<double> grad = rotateYXZ.grad();
            jacobians[0][0] = grad[0]; jacobians[0][1] = grad[1]; jacobians[0][2] = grad[2];
        }
        if (jacobians && jacobians[1]) { // jacobians for translation
            std::vector<double> grad = translate.grad();
            jacobians[1][0] = grad[0]; jacobians[1][1] = grad[1]; jacobians[1][2] = grad[2];
        }
        if (jacobians && jacobians[2]) { // jacobians for scale
            std::vector<double> grad = scale.grad();
            jacobians[2][0] = grad[0];
        }
        return true;
    }
};

struct IdenExprCostCost2DPoint : public ceres::CostFunction {
    CriterionPointToPoint2D mCriterion;
    Tensor3                 mSource;
    glm::dmat4              mPVM;

    IdenExprCostCost2DPoint(const Constraint2D &constraint,
                            const Tensor3 &tvie, int index,
                            const glm::dmat4 &pvm)
        : mCriterion(constraint.data, constraint.weight)
        , mSource(tvie, index, 3)
        , mPVM(pvm) {
        mutable_parameter_block_sizes()->clear();
        mutable_parameter_block_sizes()->push_back(FaceDB::NumDimIden());
        mutable_parameter_block_sizes()->push_back(FaceDB::NumDimExpr());
        mutable_parameter_block_sizes()->push_back(1);
        set_num_residuals(1);
    }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        // bilinear
        Tensor3 tv1e;
        Tensor3 tvi1;
        Tensor3 tv11;
        mSource.mulVec<1>(params[0], tv1e);
        mSource.mulVec<2>(params[1], tvi1);
        tv1e.mulVec<2>   (params[1], tv11);
        glm::dvec4 P = { *tv11.data(0), *tv11.data(1), *tv11.data(2), 1.0 };
        // for point
        Scale   scale(params[2][0]);
        Project project(mPVM);
        Sequential seq;
        seq.addModule(&scale);
        seq.addModule(&project);
        auto Q = seq.forward(P);
        auto point = glm::dvec2 { Q.x, Q.y };
        residuals[0] = mCriterion.forward(point);
        auto grad2   = mCriterion.backward(point);
        auto gradIn  = glm::dvec4 {grad2.x, grad2.y, 0., 0.};
        gradIn = seq.backward(gradIn);
        if (jacobians && jacobians[0]) { // jacobians for iden
            Tensor3 grad;
            tvi1.mulVec<0>(glm::value_ptr(gradIn), grad);
            for (int i = 0; i < grad.shape(1); ++i) {
                jacobians[0][i] = *grad.data(0, i);
            }
        }
        if (jacobians && jacobians[1]) { // jacobians for expr
            Tensor3 grad;
            tv1e.mulVec<0>(glm::value_ptr(gradIn), grad);
            for (int i = 0; i < grad.shape(2); ++i) {
                jacobians[1][i] = *grad.data(0, 0, i);
            }
        }
        if (jacobians && jacobians[2]) { // jacobians for scale
            std::vector<double> grad = scale.grad();
            jacobians[2][0] = grad[0];
        }
        return true;
    }
};

struct RegTerm : public ceres::CostFunction {
    int    mLength;
    double mWeight;

    RegTerm(int length, double weight) : mLength(length), mWeight(weight) {
        mutable_parameter_block_sizes()->clear();
        mutable_parameter_block_sizes()->push_back(length);
        set_num_residuals(1);
    }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        residuals[0] = 0.0;
        for (int i = 0; i < mLength; ++i) {
            residuals[0] += mWeight * params[0][i] * params[0][i];
        }
        if (jacobians && jacobians[0]) {
            for (int i = 0; i < mLength; ++i) {
                jacobians[0][i] = mWeight * 2.0 * params[0][i];
            }
        }
        return true;
    }
};

struct PoseScaleCost2DLine : public ceres::SizedCostFunction<1, 3, 3, 1> {
    std::vector<glm::dvec2> mLandmarkContour;
    double                  mWeight;
    glm::dvec4              mSource;
    glm::dmat4              mPVM;
    
    PoseScaleCost2DLine(const std::vector<glm::dvec2> &landmarkContour, double weight,
                        const glm::dvec3   &source3d,
                        const glm::dmat4   &pvm)
        : mLandmarkContour(landmarkContour)
        , mWeight(weight)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {}
    
    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        // from parameter
        RotateYXZ rotateYXZ(params[0][0], params[0][1], params[0][2]);
        Translate translate(params[1][0], params[1][1], params[1][2]);
        Scale     scale(params[2][0]);
        Project   project(mPVM);
        Sequential seq;
        seq.addModule(&scale);
        seq.addModule(&rotateYXZ);
        seq.addModule(&translate);
        seq.addModule(&project);
        auto Q       = seq.forward(mSource);
        auto point   = glm::dvec2{Q.x, Q.y};

        double distance = 100000;
        snow::double2 grad2;
        int index = 0;
        /* find line */ {
            for (int k = 0; k < mLandmarkContour.size() - 1; ++k) {
                double dist = PointToLine2D::sqrDistance(
                                    point.x, point.y,
                                    mLandmarkContour[k].x, mLandmarkContour[k].y,
                                    mLandmarkContour[k + 1].x, mLandmarkContour[k + 1].y) * mWeight;
                if (dist < distance) {
                    index = k;
                    distance = dist;
                    grad2 = PointToLine2D::diffSqrDistance(
                                    point.x, point.y,
                                    mLandmarkContour[k].x, mLandmarkContour[k].y,
                                    mLandmarkContour[k + 1].x, mLandmarkContour[k + 1].y) * mWeight;
                }
            }
        }

        auto gradIn = glm::dvec4 {grad2.x, grad2.y, 0., 0.};
        if (index == 0 || index + 2 == mLandmarkContour.size()) {
            distance = 0;
            gradIn = glm::dvec4{0, 0, 0, 0};
        }
        residuals[0] = distance;
        seq.backward(gradIn);
        if (jacobians && jacobians[0]) { // jacobians for rotation
            std::vector<double> grad = rotateYXZ.grad();
            jacobians[0][0] = grad[0]; jacobians[0][1] = grad[1]; jacobians[0][2] = grad[2];
        }
        if (jacobians && jacobians[1]) { // jacobians for translation
            std::vector<double> grad = translate.grad();
            jacobians[1][0] = grad[0]; jacobians[1][1] = grad[1]; jacobians[1][2] = grad[2];
        }
        if (jacobians && jacobians[2]) { // jacobians for scale
            std::vector<double> grad = scale.grad();
            jacobians[2][0] = grad[0];
        }
        return true;
    }
};
