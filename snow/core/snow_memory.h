#pragma once
#include <list>
#include <stdlib.h>
#include <stdint.h>
#include <stdexcept>
#include <algorithm>

#define MEMORY_ALIGNMENT 32

namespace snow {

/* aligned memory allocate and free */

void *_aligned_malloc(uint32_t size, int alignment);
void _aligned_free(void *ptr);

template <typename T> inline T *  alignedMalloc(uint32_t number, int alignment=MEMORY_ALIGNMENT) {return (T*)snow::_aligned_malloc(sizeof(T) * number, alignment); }
template <typename T> inline void alignedFree(T *ptr) { snow::_aligned_free((void *)ptr); }


/* fast memory pool */

struct Block {
    uint8_t     *mData;
    uint32_t     mSize;
    uint8_t      mCount;
    size_t       mCurPos;
    Block(uint32_t blockSize=32768)
        : mData(new uint8_t[blockSize]), mSize(blockSize), mCount(0), mCurPos(0) {}
    ~Block() { delete[] mData; }

    static size_t actualSize(size_t size) { 
        static const int pointer_size  = sizeof(void *);
        return size + (size_t)MEMORY_ALIGNMENT - 1 + pointer_size;
    }

    void *alloc(size_t size) {
        static const uintptr_t pointer_size  = (uintptr_t)sizeof(void *);
        const int alignment_1                = MEMORY_ALIGNMENT - 1;

        size = Block::actualSize(size);
        if (mCurPos + size > mSize) return nullptr;
        uintptr_t start = mCurPos + pointer_size;
        uintptr_t align = (start + alignment_1) & (~(alignment_1));

        // record block
        *((Block **)(align - pointer_size)) = (Block *)this;
        mCount += 1;
        mCurPos += size;
    }

    bool free(void *ptr) {
        if (ptr == nullptr) return;
        Block *raw = *((Block **) ( (uintptr_t)ptr - (uintptr_t)sizeof(void *) ));
        if (raw == this) mCount -= 1;
        return mCount == 0;
    }
};

class MemoryArena {
private:
    Block                  *mCurBlockPtr;
    uint32_t                mBlockSize;
    uint32_t                mBlockAlignment;
    std::list<Block *>      mUsedBlocks;
    std::list<Block *>      mAvailableBlocks;

    void _sortAvailable() {
        std::sort(mAvailableBlocks.begin(), mAvailableBlocks.end(), [](const Block *a, const Block *b) -> bool {
            return a->mSize < b->mSize;
        });
    }

public:
    MemoryArena(uint32_t blockSize=32768, int blockAlignment=MEMORY_ALIGNMENT)
        : mCurBlockPtr(nullptr)
        , mBlockSize(blockSize), mBlockAlignment(blockAlignment)
        , mUsedBlocks(0),        mAvailableBlocks(0) {}

    void *alloc(uint32_t size) {
        // round size to alignment
        size = (size + mBlockAlignment - 1) & ~(mBlockAlignment - 1);
        void * ret = nullptr;
        if (!mCurBlockPtr || !(ret = mCurBlockPtr->alloc(size))) {
            // store in used
            if (mCurBlockPtr != nullptr) mUsedBlocks.push_back(mCurBlockPtr);
            mCurBlockPtr = nullptr;

            // find in available blocks
            for (auto iter = mAvailableBlocks.begin(); iter != mAvailableBlocks.end(); ++iter) {
                if (ret = (*iter)->alloc(size)) {
                    mCurBlockPtr = (*iter);
                    mAvailableBlocks.erase(iter);
                    break;
                }
            }
            
            // still not found
            if (!mCurBlockPtr) mCurBlockPtr = new Block(std::max(Block::actualSize(size), (size_t)mBlockSize));
        }
        if (!ret)
            void *ret = mCurBlockPtr->alloc(size);
        return ret;
    }

    template <typename T>
    T *alloc(uint32_t count=1) {
        T *ret = (T*)this->alloc(count * sizeof(T));
        for (uint32_t i = 0; i < count; ++i)
            new (&ret[i]) T();
        return ret;
    }

    void reset() {
        mCurBlockPos = 0;
        mAvailableBlocks.splice(mAvailableBlocks.begin(), mUsedBlocks);
        _sortAvailable();
    }

};

}

#undef MEMORY_ALIGNMENT