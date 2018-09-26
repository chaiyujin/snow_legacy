#pragma once
#include <vector>
#include <stdint.h>

struct ModelFrame {
    int64_t mTimestamp;
    double   mRotation[3];
    double   mTranslation[3];
    double   mScale;
    std::vector<double> mIden;
    std::vector<double> mExpr;

    void print() {
        printf("time: %ld\n", this->mTimestamp);
        printf("rotate: %f %f %f\n", this->mRotation[0], this->mRotation[1], this->mRotation[2]);
        printf("translate: %f %f %f\n", this->mTranslation[0], this->mTranslation[1], this->mTranslation[2]);
        printf("scale: %f\n", this->mScale);
        printf("Iden:\n");
        for (size_t i = 0; i < this->mIden.size(); ++i) {
            printf("  %f\t", this->mIden[i]);
            if (i % 5 == 4) printf("\n");
        }
        if (this->mIden.size() % 5 != 0) printf("\n");
        printf("Expr:\n");
        for (size_t i = 0; i < this->mExpr.size(); ++i) {
            printf("  %f\t", this->mExpr[i]);
            if (i % 5 == 4) printf("\n");
        }
        if (this->mExpr.size() % 5 != 0) printf("\n");
        printf("\n");
    }
};
