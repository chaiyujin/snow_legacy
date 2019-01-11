#include "tensor.h"

snow::MemoryArena   Tensor3::gArena;
const int           Tensor3::Alignment = 32 / sizeof(double);

Tensor3::Tensor3()
    : mDataPtr(nullptr), mIsSub(false)
    , mSize(0), mMemSize(0)
    , mShape({0, 0, 0}), mMemShape({0, 0, 0}) {}
Tensor3::Tensor3(const Tensor3 &b)
    : mDataPtr(b.mDataPtr), mIsSub(b.mIsSub)
    , mSize(b.mSize), mMemSize(b.mMemSize)
    , mShape(b.mShape), mMemShape(b.mMemShape) {
    // copy data if not sub
    if (!mIsSub) {
        this->alloc(mShape);
        // mMemSize == b.mMemsize
        memcpy(mDataPtr, b.mDataPtr, sizeof(double) * mMemSize);
    }
}
Tensor3::Tensor3(const std::vector<int> &shape) : Tensor3() { resize(shape); }
Tensor3::Tensor3(const Tensor3 &father, int start, int len)
    : mIsSub(true) {
    // point to father
    len           = (len <= 0) ? father.mShape[0] - start : len;
    int blockSize = father.mMemShape[1] * father.mMemShape[2];
    mDataPtr      = father.mDataPtr + start * blockSize;
    mShape        = father.mShape;    mShape[0]    = len;
    mMemShape     = father.mMemShape; mMemShape[0] = len;
    mSize         = len * mShape[1] * mShape[2];
    mMemSize      = len * blockSize;
}
Tensor3::~Tensor3() {
    if (!mIsSub) this->free();
}

const Tensor3 &Tensor3::operator=(const Tensor3 &b) {
    if (mIsSub) throw std::runtime_error("[Tensor3]: operator = not allowed for sub-tensor!\n");
    this->free();
    mShape = b.mShape;
    this->alloc(mShape);
    // mMemSize == b.mMemsize
    memcpy(mDataPtr, b.mDataPtr, sizeof(double) * mMemSize);
}

void Tensor3::resize(const std::vector<int> & shape) {
    static const int align_1 = Alignment - 1;
    if (shape.size() != 3) throw std::runtime_error("[Tensor3]: resize() shape is not 3 dims!\n");
    if (mIsSub) throw std::runtime_error("[Tensor3]: resize() not allowed for sub-tensor!\n");

    mShape = shape;
    mSize  = mShape[0] * mShape[1] * mShape[2];

    /* check if it is necessary to alloc new memory */ {
        std::vector<int> tmp  = shape;
        alignShape(tmp);
        if (tmp[0] * tmp[1] * tmp[2] != mMemSize) {
            this->free();
            this->alloc(shape);
        }
        else {
            mMemShape = tmp;
        }
    }
    // printf("resize done real  shape %d %d %d\n", mShape[0], mShape[1], mShape[2]);
    // printf("resize done align shape %d %d %d\n", mMemShape[0], mMemShape[1], mMemShape[2]);
}