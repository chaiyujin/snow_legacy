
#pragma once
#include <ceres/ceres.h>
#include <snow.h>
#include "modules.h"
#include "../tools/math_tools.h"

template <typename T>
struct _Constraint {
    T      data;
    double weight;
};
typedef _Constraint<glm::dvec2> Constraint2D;
typedef _Constraint<glm::dvec3> Constraint3D;

/** criterion for 2d point
 * pointPtr   != nullptr -> the point2point criterion
 * contourPtr != nullptr -> the point2line  criterion
 * */
inline glm::dvec4 criterion2D(double *residuals,
                              const glm::dvec2 &inputPoint,
                              const CriterionPointToPoint2D *pointPtr,
                              const std::vector<glm::dvec2> *contourPtr, double weightContour) {
    if (pointPtr) {
        residuals[0] = pointPtr->forward(inputPoint);
        auto grad2   = pointPtr->backward(inputPoint);
        return glm::dvec4 {grad2.x, grad2.y, 0., 0.};
    }
    if (contourPtr) {
        const std::vector<glm::dvec2> &lms = *contourPtr;
        double distance = 100000;
        int index = 0;
        /* find line */ 
        for (int k = 0; k < lms.size() - 1; ++k) {
            double dist2Line  = PointToLine2D::sqrDistance (inputPoint.x, inputPoint.y, lms[k].x, lms[k].y, lms[k + 1].x, lms[k + 1].y);
            double dist2Point = PointToPoint2D::sqrDistance(inputPoint.x, inputPoint.y, lms[k].x, lms[k].y) +
                                PointToPoint2D::sqrDistance(inputPoint.x, inputPoint.y, lms[k + 1].x, lms[k + 1].y);
            double dist = dist2Line + dist2Point;
            if (dist < distance) {
                index = k;
                distance = dist;
                residuals[0] = dist2Line * weightContour;
            }
        }

        auto grad2 = PointToLine2D::diffSqrDistance(
                inputPoint.x, inputPoint.y,
                lms[index].x, lms[index].y,
                lms[index + 1].x, lms[index + 1].y) * weightContour;

        glm::dvec4 gradIn = {grad2.x, grad2.y, 0., 0.};
        if (index == 0 || index + 2 == lms.size()) {
            distance = 0;
            gradIn = glm::dvec4{0, 0, 0, 0};
        }
        return gradIn;
    }
    return glm::dvec4(0.0);
}

#define PoseScaleForward(iR, iT, iS, PVM, Source) \
    RotateYXZ  rotateYXZ(params[iR][0], params[iR][1], params[iR][2]);\
    Translate  translate(params[iT][0], params[iT][1], params[iT][2]);\
    Scale      scale    (params[iS][0]);\
    Project    project  (PVM);\
    Sequential seq;\
    seq.addModule(&scale);\
    seq.addModule(&rotateYXZ);\
    seq.addModule(&translate);\
    seq.addModule(&project);\
    auto Q       = seq.forward(Source);\
    auto point   = glm::dvec2{Q.x, Q.y};
#define PoseScaleBackward(iR, iT, iS, GradIn) \
    seq.backward(GradIn);\
    if (jacobians && jacobians[iR]) {\
        std::vector<double> grad = rotateYXZ.grad();\
        jacobians[iR][0] = grad[0]; jacobians[iR][1] = grad[1]; jacobians[iR][2] = grad[2];\
    }\
    if (jacobians && jacobians[iT]) {\
        std::vector<double> grad = translate.grad();\
        jacobians[iT][0] = grad[0]; jacobians[iT][1] = grad[1]; jacobians[iT][2] = grad[2];\
    }\
    if (jacobians && jacobians[iS]) {\
        std::vector<double> grad = scale.grad();\
        jacobians[iS][0] = grad[0];\
    }

struct PoseScaleCost2D : public ceres::SizedCostFunction<1, 3, 3, 1> {
    CriterionPointToPoint2D *mPointPtr;
    std::vector<glm::dvec2> *mContourPtr;
    double                   mWeightPoint;
    double                   mWeightContour;
    glm::dvec4               mSource;
    glm::dmat4               mPVM;
    
    PoseScaleCost2D(const glm::dvec2 *pointConstraint, double weightPoint,
                    const std::vector<glm::dvec2> *contourConstraint, double weightContour,
                    const glm::dvec3 &source3d, const glm::dmat4 &pvm)
        : mPointPtr(nullptr)
        , mContourPtr(nullptr)
        , mWeightPoint(weightPoint)
        , mWeightContour(weightContour)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {
        if (pointConstraint) {
            mPointPtr = new CriterionPointToPoint2D(*pointConstraint, weightPoint);
        }
        if (contourConstraint) {
            mContourPtr = new std::vector<glm::dvec2>();
            *mContourPtr = *contourConstraint;
        }
        if ((pointConstraint && contourConstraint) || (!pointConstraint && !contourConstraint)) {
            throw std::runtime_error("only point constraint or only contour constraint at once.");
        }
    }
    ~PoseScaleCost2D() { delete mPointPtr; delete mContourPtr; }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        // from parameter
        PoseScaleForward(0, 1, 2, mPVM, mSource);
        glm::dvec4 gradIn = criterion2D(residuals, point, mPointPtr, mContourPtr, mWeightContour);
        PoseScaleBackward(0, 1, 2, gradIn);
        return true;
    }
};

#undef PoseScaleForward
#undef PoseScaleBackward

#define IdenExprScaleForward(iI, iE, iS, CoreTensor, PVM)\
    Tensor3 tv1e;\
    Tensor3 tvi1;\
    Tensor3 tv11;\
    CoreTensor.mulVec<1>(params[iI], tv1e);\
    CoreTensor.mulVec<2>(params[iE], tvi1);\
    tv1e.mulVec<2>      (params[iE], tv11);\
    glm::dvec4 P = { *tv11.data(0), *tv11.data(1), *tv11.data(2), 1.0 };\
    Scale   scale(params[iS][0]);\
    Project project(PVM);\
    Sequential seq;\
    seq.addModule(&scale);\
    seq.addModule(&project);\
    auto Q = seq.forward(P);\
    auto point = glm::dvec2 { Q.x, Q.y };
#define IdenExprScaleBackward(iI, iE, iS, GradIn)\
    glm::dvec4 tensorGradIn = seq.backward(GradIn);\
    if (jacobians && jacobians[iI]) {\
        Tensor3 grad;\
        tvi1.mulVec<0>(glm::value_ptr(tensorGradIn), grad);\
        for (int i = 0; i < grad.shape(1); ++i) {\
            jacobians[iI][i] = *grad.data(0, i);\
        }\
    }\
    if (jacobians && jacobians[iE]) {\
        Tensor3 grad;\
        tv1e.mulVec<0>(glm::value_ptr(tensorGradIn), grad);\
        for (int i = 0; i < grad.shape(2); ++i) {\
            jacobians[iE][i] = *grad.data(0, 0, i);\
        }\
    }\
    if (jacobians && jacobians[iS]) {\
        std::vector<double> grad = scale.grad();\
        jacobians[iS][0] = grad[0];\
    }

struct IdenExprScaleCostCost2D : public ceres::CostFunction {
    CriterionPointToPoint2D *mPointPtr;
    std::vector<glm::dvec2> *mContourPtr;
    double                   mWeightPoint;
    double                   mWeightContour;
    Tensor3                  mSource;
    glm::dmat4               mPVM;

    IdenExprScaleCostCost2D(const glm::dvec2 *pointConstraint, double weightPoint,
                            const std::vector<glm::dvec2> *contourConstraint, double weightContour,
                            const Tensor3 &tvie, int vertIndex,
                            const glm::dmat4 &pvm)
        : mPointPtr(nullptr)
        , mContourPtr(nullptr)
        , mWeightPoint(weightPoint)
        , mWeightContour(weightContour)
        , mSource(tvie, vertIndex * 3, 3)
        , mPVM(pvm) {
        if (pointConstraint) {
            mPointPtr = new CriterionPointToPoint2D(*pointConstraint, weightPoint);
        }
        if (contourConstraint) {
            mContourPtr = new std::vector<glm::dvec2>();
            *mContourPtr = *contourConstraint;
        }
        if ((pointConstraint && contourConstraint) || (!pointConstraint && !contourConstraint)) {
            throw std::runtime_error("only point constraint or only contour constraint at once.");
        }
        mutable_parameter_block_sizes()->clear();
        mutable_parameter_block_sizes()->push_back(FaceDB::NumDimIden());
        mutable_parameter_block_sizes()->push_back(FaceDB::NumDimExpr());
        mutable_parameter_block_sizes()->push_back(1);
        set_num_residuals(1);
    }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        IdenExprScaleForward(0, 1, 2, mSource, mPVM);
        glm::dvec4 gradIn = criterion2D(residuals, point, mPointPtr, mContourPtr, mWeightContour);
        IdenExprScaleBackward(0, 1, 2, gradIn);
        return true;
    }
};

#undef IdenExprScaleForward
#undef IdenExprScaleBackward

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

