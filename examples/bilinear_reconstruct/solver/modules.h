#pragma once
#include <snow.h>
#include "glm_grad.h"

/****************************************************
 *                        Module                    *
 ****************************************************/

class _Module {
public:
    virtual glm::dvec4 forward( const glm::dvec4 &input)  = 0;
    virtual glm::dvec4 backward(const glm::dvec4 &gradIn) = 0;
};

class Sequential {
    std::vector<_Module *>  mModuleList;
public:
    Sequential() {}
    void addModule(_Module *m) { mModuleList.push_back(m); }
    glm::dvec4 forward(const glm::dvec4 &input) {
        glm::dvec4 tmp = input;
        for (size_t i = 0; i < mModuleList.size(); ++i) {
            tmp = mModuleList[i]->forward(tmp);
        }
        return tmp;
    }
    glm::dvec4 backward(const glm::dvec4 &gradIn) {
        glm::dvec4 tmp = gradIn;
        for (int i = (int)mModuleList.size() - 1; i >= 0; --i) {
            tmp = mModuleList[i]->backward(tmp);
        }
        return tmp;
    }
};

class Project : public _Module {
protected:
    glm::dmat4 mMat;
    glm::dvec4 mBeforeDiv;
public:
    Project(const glm::dmat4 &mat) : mMat(mat), mBeforeDiv(0.0) {}
    virtual glm::dvec4 forward(const glm::dvec4 &input) {
        mBeforeDiv = mMat * input;
        return mBeforeDiv / mBeforeDiv.w;
    }
    virtual glm::dvec4 backward(const glm::dvec4 &gradIn) {
        double iw = 1.0 / mBeforeDiv.w;
        double iw2 = iw * iw;
        glm::dvec4 tmp = {
            gradIn.x * iw, gradIn.y * iw, gradIn.z * iw,
            -iw2 * (gradIn.x * mBeforeDiv.x + gradIn.y * mBeforeDiv.y + gradIn.z * mBeforeDiv.z)
        };
        return glm::transpose(mMat) * tmp;
    }
};

class Transform : public _Module {
protected:
    glm::dmat4 mMat;
    glm::dvec4 mInput;
    glm::dvec4 mGradIn;
public:
    Transform(const glm::dmat4 &mat) : mMat(mat), mInput(0.0), mGradIn(0.0) {}
    virtual glm::dvec4 forward(const glm::dvec4 &input)   { mInput = input;   return mMat * input;                   }
    virtual glm::dvec4 backward(const glm::dvec4 &gradIn) { mGradIn = gradIn; return glm::transpose(mMat) * gradIn;  }
};

class Translate : public Transform {
public:
    Translate(double tx, double ty, double tz) : Transform(glm::translate(glm::dmat4(1.0), glm::dvec3(tx, ty, tz))) {}
    std::vector<double> grad() const {
        return { mInput.w * mGradIn.x, mInput.w * mGradIn.y, mInput.w * mGradIn.z };
    }
};

class RotateYXZ : public Transform {
private:
    glm::dmat4 Ry, dRy;
    glm::dmat4 Rx, dRx;
    glm::dmat4 Rz, dRz;
public:
    RotateYXZ(double ry, double rx, double rz) : Transform(glm::dmat4(1.0)) {
        Ry = glm::eulerAngleY(ry); dRy = glm::dEulerAngleY(ry);
        Rx = glm::eulerAngleX(rx); dRx = glm::dEulerAngleX(rx);
        Rz = glm::eulerAngleZ(rz); dRz = glm::dEulerAngleZ(rz);
        mMat = Ry * Rx * Rz;
    }
   std::vector<double> grad() const {
        double dry = glm::dot(mGradIn, dRy * Rx * Rz * mInput);
        double drx = glm::dot(glm::transpose(Ry) * mGradIn, dRx * Rz * mInput);
        double drz = glm::dot(glm::transpose(Rx) * glm::transpose(Ry) * mGradIn, dRz * mInput);
        return { dry, drx, drx };
    }
};

class Scale : public Transform {
private:
public:
    Scale(double s) : Transform(glm::scale(glm::dmat4(1.0), glm::dvec3(s, s, s))) {}
    std::vector<double> grad() const {
        return { mInput.x * mGradIn.x + mInput.y * mGradIn.y + mInput.z * mGradIn.z };
    }
};

/****************************************************
 *                      Criterion                   *
 ****************************************************/

/* sqr distance between points */
class CriterionPointToPoint2D {
private:
    glm::dvec2  mPoint;
    double      mWeight;
public:
    CriterionPointToPoint2D(const glm::dvec2 &point, double weight) : mPoint(point), mWeight(weight) {}
    double     forward(const glm::dvec2 &input)  const { double A = (input.x - mPoint.x), B = (input.y - mPoint.y); return (A * A + B * B) * mWeight;               }
    glm::dvec2 backward(const glm::dvec2 &input) const { double A = (input.x - mPoint.x), B = (input.y - mPoint.y); return { 2.0 * mWeight * A, 2.0 * mWeight * B };}
};

/* sqr distance between point and line */
class CriterionPointToLine2D {
private:
    glm::dvec2  mPoint0, mPoint1;
    double      mWeight;
public:
    CriterionPointToLine2D(const glm::dvec2 &p0, const glm::dvec2 &p1, double weight) : mPoint0(p0), mPoint1(p1), mWeight(weight) {}
    double forward(const glm::dvec2 &input) const {
        double A = mPoint1.y - mPoint0.y;
        double B = mPoint0.x - mPoint1.x;
        double C = mPoint1.x * mPoint0.y - mPoint0.x - mPoint1.y;
        double f = (A * input.x + B * input.y + C);
        return mWeight * (f * f) / (A * A + B * B);
    }
    glm::dvec2 backward(const glm::dvec2 &input) const {
        double A = mPoint1.y - mPoint0.y;
        double B = mPoint0.x - mPoint1.x;
        double C = mPoint1.x * mPoint0.y - mPoint0.x - mPoint1.y;
        double f = (A * input.x + B * input.y + C);
        double Factor = mWeight * 2.0 * f / (A * A + B * B);
        return { Factor * A, Factor * B };
    }
};
