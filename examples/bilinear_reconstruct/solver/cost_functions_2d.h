
#pragma once
#include <ceres/ceres.h>
#include <snow.h>
#include <map>
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
inline std::pair<double, glm::dvec4>criterion2D(
                              const glm::dvec2 &inputPoint,
                              const CriterionPointToPoint2D *pointPtr,
                              const std::vector<glm::dvec2> *contourPtr, double weightContour) {
    if (pointPtr) {
        double loss  = pointPtr->forward(inputPoint);
        auto grad2   = pointPtr->backward(inputPoint);
        return {loss, glm::dvec4{grad2.x, grad2.y, 0., 0.}};
    }
    else if (contourPtr) {
        const std::vector<glm::dvec2> &lms = *contourPtr;
        double distance = 100000;
        double loss = 0.0;
        int index = 0;
        snow::double2 grad2;
        /* find line */ 
        for (int k = 0; k < lms.size() - 1; ++k) {
            double dist2Line  = PointToLine2D::sqrDistance(inputPoint.x, inputPoint.y, lms[k].x, lms[k].y, lms[k+1].x, lms[k+1].y);
            double dist = dist2Line;
            if (dist < distance) {
                index = k;
                distance = dist;
                loss = dist * weightContour;
                
                grad2 = PointToLine2D::diffSqrDistance(
                                inputPoint.x, inputPoint.y,
                                lms[k].x, lms[k].y,
                                lms[k+1].x, lms[k+1].y) * weightContour;
            }
        }

        return {loss, glm::dvec4{grad2.x, grad2.y, 0., 0.}};
    }
    else
        throw std::runtime_error("error contour2d.");
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
        auto res = criterion2D(point, mPointPtr, mContourPtr, mWeightContour);
        residuals[0] = res.first;
        glm::dvec4 gradIn = res.second;
        PoseScaleBackward(0, 1, 2, gradIn);
        return true;
    }
};

#undef PoseScaleForward
#undef PoseScaleBackward

struct IdenExprScaleCost2D : public ceres::CostFunction {
    CriterionPointToPoint2D *mPointPtr;
    std::vector<glm::dvec2> *mContourPtr;
    double                   mWeightPoint;
    double                   mWeightContour;
    Tensor3                  mCore;
    glm::dmat4               mPVM;

    IdenExprScaleCost2D(const glm::dvec2 *pointConstraint, double weightPoint,
                        const std::vector<glm::dvec2> *contourConstraint, double weightContour,
                        const Tensor3 &tvie, int vertIndex,
                        const glm::dmat4 &pvm)
        : mPointPtr(nullptr)
        , mContourPtr(nullptr)
        , mWeightPoint(weightPoint)
        , mWeightContour(weightContour)
        , mCore(tvie, vertIndex * 3, 3)
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
        mutable_parameter_block_sizes()->push_back(FaceDB::LengthIdentity   - 1);
        mutable_parameter_block_sizes()->push_back(FaceDB::LengthExpression - 1);
        mutable_parameter_block_sizes()->push_back(1);
        set_num_residuals(1);
    }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        const int iI = 0, iE = 1, iS = 2;
        std::vector<double> iden(FaceDB::LengthIdentity);
        std::vector<double> expr(FaceDB::LengthExpression);
        iden[0] = expr[0] = 1.0;
        memcpy(&iden[1], params[iI], sizeof(double) * (FaceDB::LengthIdentity   - 1));
        memcpy(&expr[1], params[iE], sizeof(double) * (FaceDB::LengthExpression - 1));
        // bilinear
        Tensor3 tv1e;
        Tensor3 tvi1;
        Tensor3 tv11;
        mCore.mulVec<1>(iden.data(), tv1e);
#ifdef PARAMETER_FACS
        auto facs = ExprParameter::FACS2Expr(expr.data());
        mCore.mulVec<2>(facs.data(), tvi1);
        tv1e.mulVec<2> (facs.data(), tv11);
#else
        mCore.mulVec<2>(expr.data(), tvi1);
        tv1e.mulVec<2> (expr.data(), tv11);
#endif
        glm::dvec4 P = { *tv11.data(0), *tv11.data(1), *tv11.data(2), 1.0 };
        Scale   scale(params[iS][0]);
        Project project(mPVM);
        Sequential seq;
        seq.addModule(&scale);
        seq.addModule(&project);
        auto Q = seq.forward(P);
        auto point = glm::dvec2 { Q.x, Q.y };

        auto res = criterion2D(point, mPointPtr, mContourPtr, mWeightContour);
        residuals[0] = res.first;
        glm::dvec4 gradIn = res.second;

        glm::dvec4 tensorGradIn = seq.backward(gradIn);
        if (jacobians && jacobians[iI]) {
            Tensor3 grad;
            tvi1.mulVec<0>(glm::value_ptr(tensorGradIn), grad);
            for (int i = 1; i < grad.shape(1); ++i) {
                jacobians[iI][i - 1] = *grad.data(0, i);
            }
        }
        if (jacobians && jacobians[iE]) {
            Tensor3 grad;
            tv1e.mulVec<0>(glm::value_ptr(tensorGradIn), grad);
            std::vector<double> exprGrad(grad.shape(2));
            for (int i = 0; i < grad.shape(2); ++i) exprGrad[i] = *grad.data(0, 0, i);
            exprGrad[0] = 0;
#ifdef PARAMETER_FACS
            auto facsGrad = ExprParameter::DiffFACS2Expr(exprGrad.data());
            for (int i = 1; i < facsGrad.size(); ++i) {
                jacobians[iE][i - 1] = facsGrad[i];
            }
#else
            for (int i = 1; i < grad.shape(2); ++i) {
                jacobians[iE][i - 1] = exprGrad[i];
            }
#endif
        }
        if (jacobians && jacobians[iS]) {
            std::vector<double> grad = scale.grad();
            jacobians[iS][0] = grad[0];
        }
        return true;
    }
};


struct IdenExprScalePoseCost2D : public ceres::CostFunction {    
    CriterionPointToPoint2D *mPointPtr;
    std::vector<glm::dvec2> *mContourPtr;
    double                   mWeightPoint;
    double                   mWeightContour;
    Tensor3                  mCore;
    glm::dmat4               mPVM;

    IdenExprScalePoseCost2D(const glm::dvec2 *pointConstraint, double weightPoint,
                            const std::vector<glm::dvec2> *contourConstraint, double weightContour,
                            const Tensor3 &tvie, int vertIndex,
                            const glm::dmat4 &pvm)
        : mPointPtr(nullptr)
        , mContourPtr(nullptr)
        , mWeightPoint(weightPoint)
        , mWeightContour(weightContour)
        , mCore(tvie, vertIndex * 3, 3)
        , mPVM(pvm) {
        if (pointConstraint)    { mPointPtr   = new CriterionPointToPoint2D(*pointConstraint, weightPoint); }
        if (contourConstraint)  { mContourPtr = new std::vector<glm::dvec2>(); *mContourPtr = *contourConstraint; }
        if ((pointConstraint && contourConstraint) || (!pointConstraint && !contourConstraint))
            throw std::runtime_error("only point constraint or only contour constraint at once.");
        mutable_parameter_block_sizes()->clear();
        mutable_parameter_block_sizes()->push_back(FaceDB::LengthIdentity   - 1);
        mutable_parameter_block_sizes()->push_back(FaceDB::LengthExpression - 1);
        mutable_parameter_block_sizes()->push_back(1);
        mutable_parameter_block_sizes()->push_back(3);
        mutable_parameter_block_sizes()->push_back(3);
        set_num_residuals(1);
    }

    virtual bool Evaluate(double const * const * params, double *residuals, double **jacobians) const {
        const int iI = 0, iE = 1, iS = 2, iR = 3, iT = 4;
        std::vector<double> iden(FaceDB::LengthIdentity);
        std::vector<double> expr(FaceDB::LengthExpression);
        iden[0] = expr[0] = 1.0;
        memcpy(&iden[1], params[iI], sizeof(double) * (FaceDB::LengthIdentity   - 1));
        memcpy(&expr[1], params[iE], sizeof(double) * (FaceDB::LengthExpression - 1));
        // bilinear
        Tensor3 tv1e;
        Tensor3 tvi1;
        Tensor3 tv11;
        mCore.mulVec<1>(iden.data(), tv1e);
#ifdef PARAMETER_FACS
        auto facs = ExprParameter::FACS2Expr(expr.data());
        mCore.mulVec<2>(facs.data(), tvi1);
        tv1e.mulVec<2> (facs.data(), tv11);
#else
        mCore.mulVec<2>(expr.data(), tvi1);
        tv1e.mulVec<2> (expr.data(), tv11);
#endif
        glm::dvec4 P = { *tv11.data(0), *tv11.data(1), *tv11.data(2), 1.0 };
        // mats
        Scale      scale    (params[iS][0]);
        RotateYXZ  rotateYXZ(params[iR][0], params[iR][1], params[iR][2]);
        Translate  translate(params[iT][0], params[iT][1], params[iT][2]);
        Project    project  (mPVM);
        Sequential seq;
        seq.addModule(&scale);
        seq.addModule(&rotateYXZ);
        seq.addModule(&translate);
        seq.addModule(&project);
        auto Q       = seq.forward(P);
        auto point   = glm::dvec2{Q.x, Q.y};

        auto res = criterion2D(point, mPointPtr, mContourPtr, mWeightContour);
        residuals[0] = res.first;
        glm::dvec4 gradIn = res.second;
        
        glm::dvec4 tensorGradIn = seq.backward(gradIn);
        if (jacobians && jacobians[iI]) {
            Tensor3 grad;
            tvi1.mulVec<0>(glm::value_ptr(tensorGradIn), grad);
            for (int i = 1; i < grad.shape(1); ++i) {
                jacobians[iI][i - 1] = *grad.data(0, i);
            }
        }
        if (jacobians && jacobians[iE]) {      
            Tensor3 grad;
            tv1e.mulVec<0>(glm::value_ptr(tensorGradIn), grad);
            std::vector<double> exprGrad(grad.shape(2));
            for (int i = 0; i < grad.shape(2); ++i) exprGrad[i] = *grad.data(0, 0, i);
            exprGrad[0] = 0;
#ifdef PARAMETER_FACS
            auto facsGrad = ExprParameter::DiffFACS2Expr(exprGrad.data());
            for (int i = 1; i < facsGrad.size(); ++i) {
                jacobians[iE][i - 1] = facsGrad[i];
            }
#else
            for (int i = 1; i < grad.shape(2); ++i) {
                jacobians[iE][i - 1] = exprGrad[i];
            }
#endif
        }
        if (jacobians && jacobians[iS]) {
            std::vector<double> grad = scale.grad();
            jacobians[iS][0] = grad[0];
        }
        if (jacobians && jacobians[iR]) {
            std::vector<double> grad = rotateYXZ.grad();
            jacobians[iR][0] = grad[0]; jacobians[iR][1] = grad[1]; jacobians[iR][2] = grad[2];
        }
        if (jacobians && jacobians[iT]) {
            std::vector<double> grad = translate.grad();
            jacobians[iT][0] = grad[0]; jacobians[iT][1] = grad[1]; jacobians[iT][2] = grad[2];
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

