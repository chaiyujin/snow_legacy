#pragma once
#include <ceres/ceres.h>
#include <snow.h>
#include "../tools/distances.h"

template <typename T>
struct _Constraint {
    T      data;
    double weight;
};
typedef _Constraint<glm::dvec2> Constraint2D;
typedef _Constraint<glm::dvec3> Constraint3D;

struct Cost2DPose : public ceres::SizedCostFunction<1, 3, 3> {

    virtual bool Evaluate(
        double const * const * params,
        double *residuals,
        double **jacobians) const {
        // from parameter
        glm::dmat4 Ry = glm::eulerAngleY(params[0][0]);
        glm::dmat4 Rx = glm::eulerAngleY(params[0][1]);
        glm::dmat4 Rz = glm::eulerAngleY(params[0][2]);
        glm::dmat4 T  = glm::translate(glm::dmat4(1.0), params[1][0], params[1][1], params[1][2]);
        glm::dmat4 M  = T * Ry * Rx * Rz;
        glm::dvec4 q  = mPVM * M * mSource;
        q.x /= q.w;
        q.y /= q.w;
        residuals[0] = Point2Point2D::sqrDistance(q.x, q.y, mConstraint.data.x, mConstraint.data.y) * mConstraint.weight;
    }

    Cost2DPose(const Constraint2D &constraint,
               const glm::dvec3   &source3d,
               const glm::dmat4   &pvm)
        : mConstraint(constraint)
        , mSource(source3d.x, source3d.y, source3d.z, 1.0)
        , mPVM(pvm) {}

    Constraint2D mConstraint;
    glm::dvec4   mSource;
    glm::dmat4   mPVM;
};

