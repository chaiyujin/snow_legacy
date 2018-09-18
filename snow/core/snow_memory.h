#pragma once
#include <stdint.h>
#include <stdexcept>
#include <vector>
#include <algorithm>

#define MEMORY_ALIGNMENT 32

namespace snow {

/* aligned memory allocate and free */

void *_aligned_malloc(uint32_t size, int alignment);
void _aligned_free(void *ptr);

template <typename T> inline T *  alignedMalloc(uint32_t number, int alignment=MEMORY_ALIGNMENT) { return (T*)_aligned_malloc(sizeof(T) * number, alignment); }
template <typename T> inline void alignedFree(T *ptr) { _aligned_free((void *)ptr); }


/* fast memory pool */

struct Block {
    uint8_t     *data;
    uint32_t     size;
    Block(uint32_t blockSize=32768) : data(alignedMalloc<uint8_t>(size)), size(blockSize) {}
};

class MemoryArena {
private:
    Block                  *mCurBlockPtr;
    uintptr_t               mCurBlockPos;
    uint32_t                mBlockSize;
    uint32_t                mBlockAlignment;
    std::vector<Block *>    mUsedBlocks;
    std::vector<Block *>    mAvailableBlocks;

    void _sortAvailable() {
        std::sort(mAvailableBlocks.begin(), mAvailableBlocks.end(), [](const Block *a, const Block *b) -> bool {
            return a->size < b->size;
        });
    }

public:
    MemoryArena(uint32_t blockSize=32768, int blockAlignment=MEMORY_ALIGNMENT)
        : mCurBlockPtr(new Block(blockSize)), mCurBlockPos(0)
        , mBlockSize(blockSize), mBlockAlignment(blockAlignment)
        , mUsedBlocks(0), mAvailableBlocks(0) {}

    void *alloc(uint32_t size) {
        // round size to alignment
        size = (size + mBlockAlignment - 1) & ~(mBlockAlignment - 1);
        if (mCurBlockPos + size > mCurBlockPtr->size) {
            // not enough
            mUsedBlocks.push_back(mCurBlockPtr);
            // find in available blocks
            if (mAvailableBlocks.size() && (mAvailableBlocks.back()->size >= size)) {
                mCurBlockPtr = mAvailableBlocks.back();
                mAvailableBlocks.pop_back();
            }
            else {
                // not enough
                mCurBlockPtr = new Block(std::max(size, mBlockSize));
            }
            mCurBlockPos = 0;
        }
        void *ret = (void*)((uintptr_t)mCurBlockPtr->data + mCurBlockPos);
        mCurBlockPos += size;
        return ret;
    }

    template <typename T>
    T *alloc(uint32_t count=1) {
        T *ret = (T*)this->alloc(count * sizeof(T));
        for (uint32_t i = 0; i < count; ++i)
            new (&ret[i]) T();
        return ret;
    }

    void freeAll() {
        mCurBlockPos = 0;
        while (mUsedBlocks.size()) {
            mAvailableBlocks.push_back(mUsedBlocks.back());
            mUsedBlocks.pop_back();
        }
        _sortAvailable();
    }

};

}

#undef MEMORY_ALIGNMENT