#pragma once
#include <snow.h>
#include <string>
#include <vector>
#include "tensor.h"
#include "faceware.h"
#include "bilinear_model.h"

typedef std::vector<glm::vec3> Vertices;

class ShowModel : public snow::Model {
    void load();
public:
    ShowModel() { load(); }
    ~ShowModel() {}
    void updateFromTensor(const Tensor3 &tensor);
};

class ShowWindow : public snow::CameraWindow {
private:
    ShowModel        mGLModel;
    snow::Shader    *mShaderPtr;
    BilinearModel    mBilinearModel;
    std::vector<float> mIden;
    std::vector<float> mExpr;

public:
    ShowWindow();
    void draw();

    template <typename T>
    void updateShowParameters(T *iden, T *expr) {
        for (size_t i = 0; i < mIden.size(); ++i) {
            mIden[i] = iden[i];
            mBilinearModel.idenParameter().param()[i] = iden[i];
        }
        for (size_t i = 0; i < mExpr.size(); ++i) {
            mExpr[i] = expr[i];
            mBilinearModel.exprParameter(0).param()[i] = expr[i];
        }
        mBilinearModel.updateMesh();
        mGLModel.updateFromTensor(mBilinearModel.mesh(0));
    }
};
