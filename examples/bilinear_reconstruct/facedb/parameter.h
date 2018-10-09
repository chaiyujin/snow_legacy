#pragma once

#include <snow.h>
#include <iostream>
#include "tensor.h"
#include "facedb.h"

class Parameter {
protected:
    static snow::MemoryArena gArena;
    double * mParamPtr;
    double * mTrainPtr;
    int      mLength;
    Parameter(const Parameter &) {}
public:
    Parameter(int length)
        : mParamPtr(gArena.alloc<double>(length))
        , mTrainPtr(gArena.alloc<double>(length))
        , mLength(length) { }
    virtual ~Parameter() {
        gArena.free<double>(mParamPtr);
        gArena.free<double>(mTrainPtr);
    }
    virtual void reset()      { memset(mParamPtr, 0, sizeof(double) * mLength); memset(mTrainPtr, 0, sizeof(double) * mLength); }
    virtual void useTrained() { memcpy(mParamPtr, mTrainPtr, sizeof(double) * mLength); }

    double *param() { return mParamPtr; }
    double *train() { return mTrainPtr; }
    const double *param() const { return mParamPtr; }
    const double *train() const { return mTrainPtr; }

    void copyFrom(const Parameter *param) {
        memcpy(mParamPtr, param->mParamPtr, sizeof(double) * mLength);
        memcpy(mTrainPtr, param->mTrainPtr, sizeof(double) * mLength);
    }
};

class PoseParameter : public Parameter {
public:
#define getT(param, i) param[i]
#define getR(param, i) param[i + 3]

    static const int Length = 6;
    static const int LengthRotateYXZ = 3;
    static const int LengthTranslate = 3;
    PoseParameter(): Parameter(Length) { reset(); }
    void reset() {
        memset(mTrainPtr, 0, sizeof(double) * Length);
        memset(mParamPtr, 0, sizeof(double) * Length);
        getT(mTrainPtr, 2) = getT(mParamPtr, 2) = -0.45f;
    }

    double *rotateYXZ()                   { return &getR(mParamPtr, 0); }
    double *translate()                   { return &getT(mParamPtr, 0); }
    double *trainRotateYXZ()              { return &getR(mTrainPtr, 0); }
    double *trainTranslate()              { return &getT(mTrainPtr, 0); }
    const double *rotateYXZ()       const { return &getR(mParamPtr, 0); }
    const double *translate()       const { return &getT(mParamPtr, 0); }
    const double *trainRotateYXZ()  const { return &getR(mTrainPtr, 0); }
    const double *trainTranslate()  const { return &getT(mTrainPtr, 0); }

    void useTrainedRotateYXZ() { memcpy(&getR(mParamPtr, 0), &getR(mTrainPtr, 0), LengthRotateYXZ * sizeof(double)); }
    void useTrainedTranslate() { memcpy(&getT(mParamPtr, 0), &getT(mTrainPtr, 0), LengthTranslate * sizeof(double)); }

    glm::dmat4 matR() const { return glm::eulerAngleYXZ(getR(mParamPtr, 0), getR(mParamPtr, 1), getR(mParamPtr, 2)); }
    glm::dmat4 matT() const { return glm::translate(glm::dmat4(1.0), glm::dvec3(getT(mParamPtr, 0), getT(mParamPtr, 1), getT(mParamPtr, 2))); }

#undef getT
#undef getR
};
inline std::ostream &operator<<(std::ostream &out, const PoseParameter &param) {
    out << "rotateYXZ: [" << param.rotateYXZ()[0] << ", " << param.rotateYXZ()[1] << ", " << param.rotateYXZ()[2] << "]\n"
        << "translate: [" << param.translate()[0] << ", " << param.translate()[1] << ", " << param.translate()[2] << "]\n";
    return out;
}

class ScaleParameter: public Parameter {
public:
    static const int Length = 1;
    ScaleParameter() : Parameter(Length) { reset(); }

    void reset() {
        memset(mParamPtr, 0, sizeof(double) * Length);
        memset(mTrainPtr, 0, sizeof(double) * Length);
        mParamPtr[0] = mTrainPtr[0] = 0.01;
    }
};
inline std::ostream &operator<<(std::ostream &out, const ScaleParameter &param) {
    out << "scale: [" << param.param()[0] << "]\n";
    return out;
}

class IdenParameter : public Parameter {
public:
    static const int Length  = FaceDB::LengthIdentity;
    IdenParameter() : Parameter(Length) { reset(); }

    void reset() {
        memset(mParamPtr, 0, sizeof(double) * Length);
        memset(mTrainPtr, 0, sizeof(double) * Length);
        mParamPtr[0] = mTrainPtr[0] = 1;
    }
    
};

class ExprParameter : public Parameter {
public:
    static const int Length      = FaceDB::LengthExpression;

    ExprParameter() : Parameter(Length) { reset(); }

    void reset() {
        memset(mParamPtr, 0, sizeof(double) * Length);
        memset(mTrainPtr, 0, sizeof(double) * Length);
        mParamPtr[0] = mTrainPtr[0] = 1;
    }

    static Eigen::VectorXd FACS2Expr(const double *facs) {
        Eigen::VectorXd E_BS = Eigen::Map<const Eigen::Matrix<double, -1, 1>>(facs, FaceDB::ExprUT().cols());
#ifdef USE_50_25
        E_BS[0] = 1 - (E_BS.sum() - E_BS[0]);
#else
        E_BS[0] = 1;
#endif
        return FaceDB::ExprUT() * E_BS;
    }
    static void FACS2Expr(const double *facs, double *expr) {
        Eigen::VectorXd E_BS = Eigen::Map<const Eigen::Matrix<double, -1, 1>>(facs, FaceDB::ExprUT().cols());
#ifdef USE_50_25
        E_BS[0] = 1 - (E_BS.sum() - E_BS[0]);
#else
        E_BS[0] = 1;
#endif
        memcpy(expr, E_BS.data(), sizeof(double) * E_BS.size());
    }
    static Eigen::VectorXd DiffFACS2Expr(const double *exprGrad) {
        Eigen::VectorXd _exprGrad = Eigen::Map<const Eigen::Matrix<double, -1, 1>>(exprGrad, FaceDB::ExprUT().rows());
        Eigen::VectorXd grad = FaceDB::ExprUT().transpose() * _exprGrad;
        grad[0] = 0;
        return grad;
    }
};
