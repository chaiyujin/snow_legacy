#pragma once
#include <vector>
#include <iostream>
#include <initializer_list>
#include <assert.h>
#include <memory>

#include <snow.h>

/* 3 dim Tensor3 */
class Tensor3 {
public:

    Tensor3();
    Tensor3(const Tensor3 &b);
    Tensor3(const std::vector<int> &shape);
    Tensor3(const Tensor3 &father, int start, int len=0);
    ~Tensor3();

    bool isNull() const { return !mDataPtr; }
    void resize(const std::vector<int> & shape);
    const Tensor3 &operator=(const Tensor3 &b);

    template <typename T>
    void unfoldData(T *data, int unfold_mode, bool is_data_colmajor=true) {
        assert(unfold_mode >= 0 && unfold_mode < 3);
        if (mIsSub) {
            printf("[Tensor3 error]: You cannot set data of sub-Tensor3.");
            exit(1);
            return;
        }

        std::vector<int> indices(mShape.size());
        std::vector<int> div(mShape.size());  // idx -> i, j, k...
        std::vector<int> mul(mShape.size());  // idx <- i, j, k...
        int D = (int)indices.size();
        {
            div[mShape.size() - 1] = 1;
            for (int i = (int)mShape.size() - 2; i >= 0; --i) { div[i] = div[i + 1] * mShape[i + 1]; }
        }
        if (is_data_colmajor) {
            for (int i = 0, d=unfold_mode, v=1; i < D; ++i) {
                mul[d] = v;
                v *= mShape[d];
                d = (d + 1) % D;
            }
        }
        else snow::fatal("[Tensor3]: unfoldData() don't support row major!\n");

        double *p = mDataPtr;
        for (int c0 = 0, i = 0; c0 < mShape[0]; ++c0) {
            for (int c1 = 0; c1 < mShape[1]; ++c1, p += mMemShape[2]) {
                for (int c2 = 0; c2 < mShape[2]; ++c2, ++i) {
                    int ii = i;
                    int j = 0;
                    for (int d = 0; d < D; ++d) {
                        indices[d] = ii / div[d];
                        ii %= div[d];
                        j += indices[d] * mul[d];
                    }
                    p[c2] = data[j];
                }
            }
        }
    }

    const std::vector<int> &shape() const { return mShape; }
    const int shape(int i) const { return mShape[i]; }
    const int memShape(int i) const { return mMemShape[i]; }

    template <int Mode>
    void mulVec(const double *vec, Tensor3 &result) const;
    void mulVec(const double *vec1, const double *vec2, Tensor3 &result) const;
    void mul(double value, Tensor3 &result) const;


    void            zero()                                 { memset(mDataPtr, 0, sizeof(double) * mMemSize); }

    // double *        data()                                 { return mDataPtr; }
    // const double *  data()                           const { return mDataPtr; }
    double *        data(int d0, int d1=0, int d2=0)       { return mDataPtr + d0 * mMemShape[1] * mMemShape[2] + d1 * mMemShape[2] + d2; }
    const double *  data(int d0, int d1=0, int d2=0) const { return mDataPtr + d0 * mMemShape[1] * mMemShape[2] + d1 * mMemShape[2] + d2; }

    void            clear() { free(); }
    static void     LogMemoryArena() { gArena.log(); }
    static void     ResetMemoryArena() { gArena.reset(); }

private:
    double *            mDataPtr;
    bool                mIsSub;
    int                 mSize;
    int                 mMemSize;
    std::vector<int>    mShape;
    std::vector<int>    mMemShape;

    static snow::MemoryArena gArena;
    static const int Alignment;  // Alignment * sizeof(double) == 32

    void alignShape(std::vector<int> &shape) {
        static const int align_1 = Alignment - 1;
        for (int i = 2; i > 0; --i) {
            if (shape[i] > 1) {
                shape[i] = (shape[i] + align_1) & ~align_1;
                return;
            }
        }
        // D, 1, 1
        // shape[1] = Alignment;  // cannot change last dim
    }

    void free() {
        if (!mDataPtr) return;
        gArena.free<double>(mDataPtr);
        mShape = mMemShape = {0, 0, 0};
        mSize  = mMemSize  = 0;
        mDataPtr = nullptr;
    }

    void alloc(const std::vector<int> &shape) {
        // if 0 dim, return
        if (shape[0] == 0 || shape[1] == 0 || shape[2] == 0) return;
        mShape = shape;
        mMemShape = mShape;
        alignShape(mMemShape);
        mSize    = mShape[0]    * mShape[1]    * mShape[2];
        mMemSize = mMemShape[0] * mMemShape[1] * mMemShape[2];
        mDataPtr = gArena.alloc<double>(mMemSize);
        mIsSub   = false; // should be false before alloc
    }
};

template <>
inline void Tensor3::mulVec<0>(const double *vec, Tensor3 &result) const {
    if (!(result.mShape[0] == 1 && result.mShape[1] == mShape[1] && result.mShape[2] == mShape[2]))
        result.resize({ 1, mShape[1], mShape[2] });
    result.zero();
    for (int i = 0; i < mShape[0]; ++i) {
        for (int j = 0; j < mShape[1]; ++j) {
            for (int k = 0; k < mShape[2]; ++k) {
                *result.data(0, j, k) += *data(i, j, k) * vec[i];
            }
        }
    }
}

template <>
inline void Tensor3::mulVec<1>(const double *vec, Tensor3 &result) const {
    if (!(result.mShape[0] == mShape[0] && result.mShape[1] == 1 && result.mShape[2] == mShape[2]))
        result.resize({ mShape[0], 1, mShape[2] });
    result.zero();
    const int src12 = mMemShape[1]        * mMemShape[2];
    const int tar12 = result.mMemShape[1] * result.mMemShape[2];
    for (int i = 0, r0 = 0, c0 = 0; i < mShape[0]; ++i, r0 += tar12, c0 += src12) {
        for (int j = 0, c1= 0; j < mShape[1]; ++j, c1 += mMemShape[2]) {
            snow::addWB(&result.mDataPtr[r0], &mDataPtr[c0+c1], vec[j], mShape[2]);
        }
    }
}

template <>
inline void Tensor3::mulVec<2>(const double *vec, Tensor3 &result) const {
    if (!(result.mShape[0] == mShape[0] && result.mShape[1] == mShape[1] && result.mShape[2] == 1))
        result.resize({ mShape[0], mShape[1], 1 });
    result.zero();
    const int src12 = mMemShape[1]        * mMemShape[2];
    const int tar12 = result.mMemShape[1] * result.mMemShape[2];
    for (int i = 0, r0 = 0, c0 = 0; i < mShape[0]; ++i, r0 += tar12, c0 += src12) {
        for (int j = 0, r1 = 0, c1 = 0; j < mShape[1]; ++j, r1 += result.mMemShape[2], c1 += mMemShape[2]) {
            result.mDataPtr[r0 + r1] += snow::dot(&mDataPtr[c0 + c1], vec, mShape[2]);
        }
    }
}

inline void Tensor3::mulVec(const double *vec1, const double *vec2, Tensor3 &result) const {
    Tensor3 tmp;
    if (mShape[2] > mShape[1])  { mulVec<2>(vec2, tmp); tmp.mulVec<1>(vec1, result); }
    else                        { mulVec<1>(vec1, tmp); tmp.mulVec<2>(vec2, result); }
}

inline void Tensor3::mul(double scale, Tensor3 &result) const {
    
    if (!(result.mShape == mShape))
        result.resize(mShape);
    result.zero();
    int shape12 = mMemShape[1] * mMemShape[2];
    for (int i = 0, c0 = 0; i < mShape[0]; ++i, c0 += shape12) {
        for (int j = 0, c1 = 0; j < mShape[1]; ++j, c1 += mMemShape[2]) {
            // for (int k = 0; k < mShape[2]; ++k) {
            //     result.mDataPtr[c0 + c1 + k] = mDataPtr[c0 + c1 + k] * scale;
            // }
            snow::storeWB(&result.mDataPtr[c0 + c1], &mDataPtr[c0 + c1], scale, mShape[2]);
        }
    }
}
