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

struct PoseCost2DPoint : public ceres::SizedCostFunction<1, 3, 3> {

    virtual bool Evaluate(
        double const * const * params,
        double *residuals,
        double **jacobians) const {
        // from parameter
        RotateYXZ rotateYXZ(params[0][0], params[0][1], params[0][2]);
        Translate translate(params[1][0], params[1][1], params[1][2]);
        Project   project(mPVM);
        Net net;
        net.addModule(&rotateYXZ);
        net.addModule(&translate);
        net.addModule(&project);
        glm::dvec4 Q = net.forward(mSource);
        residuals[0] = Point2Point2D::sqrDistance(Q.x, Q.y, mConstraint.data.x, mConstraint.data.y) * mConstraint.weight;
        // std::cout << "residuals " << residuals[0] << " constraint " << mConstraint.data << " point " << Q << std::endl;
        // grad
        snow::double2 grad   = Point2Point2D::diffSqrDistance(Q.x, Q.y, mConstraint.data.x, mConstraint.data.y) * mConstraint.weight;
        glm::dvec4    gradIn = {grad.x, grad.y, 0., 0.};
        net.backward(gradIn);
        // jacobians for rotation
        if (jacobians && jacobians[0]) {
            std::vector<double> grad = rotateYXZ.grad();
            jacobians[0][0] = grad[0];
            jacobians[0][1] = grad[1];
            jacobians[0][2] = grad[2];
        }
        // jacobians for translation
        if (jacobians && jacobians[1]) {
            // w == 1
            std::vector<double> grad = translate.grad();
            jacobians[1][0] = grad[0];
            jacobians[1][1] = grad[1];
            jacobians[1][2] = grad[2];
        }
        return true;
    }

    PoseCost2DPoint(const Constraint2D &constraint,
                    const glm::dvec3   &source3d,
                    const glm::dmat4   &pvm)
        : mConstraint(constraint)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {}

    Constraint2D mConstraint;
    glm::dvec4   mSource;
    glm::dmat4   mPVM;
};


struct PoseCost2DLine : public ceres::SizedCostFunction<1, 3, 3> {
    virtual bool Evaluate(
        double const * const * params,
        double *residuals,
        double **jacobians) const {
        // from parameter
        glm::dmat4 Ry = glm::eulerAngleY(params[0][0]);
        glm::dmat4 Rx = glm::eulerAngleX(params[0][1]);
        glm::dmat4 Rz = glm::eulerAngleZ(params[0][2]);
        glm::dmat4 T  = glm::translate(glm::dmat4(1.0), glm::dvec3(params[1][0], params[1][1], params[1][2]));
        glm::dmat4 M  = T * Ry * Rx * Rz;
        glm::dvec4 P  = mPVM * M * mSource;
        glm::dvec4 Q  = operation::Project::forward(P);

        double distance = 100000;
        snow::double2 grad;
        /* find line */ {
            for (int k = 0; k < mConstraints.size() - 1; ++k) {
                double dist = PointToLine2D::sqrDistance(Q.x, Q.y, mConstraints[k].x, mConstraints[k].y, mConstraints[k + 1].x, mConstraints[k + 1].y);
                if (dist < distance) {
                    distance = dist;
                    grad = PointToLine2D::diffSqrDistance(Q.x, Q.y, mConstraints[k].x, mConstraints[k].y, mConstraints[k + 1].x, mConstraints[k + 1].y);
                }
            }
        }

        residuals[0] = distance;
        // grad
        glm::dvec4    gradIn = {grad.x, grad.y, 0., 0.};
        gradIn = operation::Project::backward(P, gradIn);
        gradIn = operation::Transform::backward(mPVM, gradIn);
        // jacobians for rotation
        if (jacobians && jacobians[0]) {
            glm::dmat4 dRy = glm::dEulerAngleY(params[0][0]);
            glm::dmat4 dRx = glm::dEulerAngleX(params[0][1]);
            glm::dmat4 dRz = glm::dEulerAngleZ(params[0][2]);

            gradIn = operation::Transform::backward(T, gradIn);
            jacobians[0][0] = glm::dot(gradIn, dRy * Rx * Rz * mSource);
            gradIn = operation::Transform::backward(Ry, gradIn);
            jacobians[0][1] = glm::dot(gradIn, dRx * Rz * mSource);
            gradIn = operation::Transform::backward(Rx, gradIn);
            jacobians[0][2] = glm::dot(gradIn, dRz * mSource);
        }
        // jacobians for translation
        if (jacobians && jacobians[1]) {
            // w == 1
            jacobians[1][0] = gradIn.x;
            jacobians[1][1] = gradIn.y;
            jacobians[1][2] = gradIn.z;
        }
        return true;
    }

    PoseCost2DLine(const std::vector<glm::dvec2> &constraints,
                   const glm::dvec3   &source3d,
                   const glm::dmat4   &pvm)
        : mConstraints(constraints)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {}

    std::vector<glm::dvec2> mConstraints;
    glm::dvec4       mSource;
    glm::dmat4       mPVM;
};
