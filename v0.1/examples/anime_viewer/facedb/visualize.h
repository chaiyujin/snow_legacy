#pragma once
#include <snow.h>
#include <string>
#include <vector>

#include "tensor.h"
#include "facedb.h"
#include "bilinear_model.h"

class ShowModel : public snow::Model {
    BilinearModel mBilinearModel;

    void load();
    void updateFromTensor(const Tensor3 &tensor);
public:
    ShowModel() { mBilinearModel.appendModel(1); mBilinearModel.prepareAllModel(); load(); }
    ~ShowModel() {}
    
    void updateIden(const std::vector<double> &iden);
    void updateExpr(const std::vector<double> &expr);

    void modify(const void *userData) {
        updateExpr(*((const std::vector<double> *)userData));
    }
};
