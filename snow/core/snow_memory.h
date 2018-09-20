#pragma once
#include <list>
#include <stdlib.h>
#include <stdint.h>
#include <stdexcept>
#include <algorithm>

#define MEMORY_ALIGNMENT 32

namespace snow {

/* aligned memory allocate and free */

inline void *_aligned_malloc(uint32_t size, int alignment) {
    if (alignment & (alignment - 1)) {
        throw std::invalid_argument("[_aligned_malloc]: alignment should be 2 ^ n.");
    }
    const int pointer_size      = sizeof(void *);
    const int required_size     = (int)size + (int)alignment - 1 + pointer_size;
    const uintptr_t alignment_1 = alignment - 1;

    void *raw = malloc(required_size);
    uintptr_t start = (uintptr_t)raw + (uintptr_t)pointer_size;
    uintptr_t align = (start + alignment_1) & (~(alignment_1));
    // store the raw address for free
    *((void **)(align - pointer_size)) = raw;
    return (void *) align;
}

inline void _aligned_free(void *ptr) {
    if (ptr == nullptr) return;
    void *raw = *((void **) ( (uintptr_t)ptr - (uintptr_t)sizeof(void *) ));
    free(raw);
}

template <typename T> inline T *  alignedMalloc(uint32_t number, int alignment=MEMORY_ALIGNMENT) {return (T*)snow::_aligned_malloc(sizeof(T) * number, alignment); }
template <typename T> inline void alignedFree(T *ptr) { snow::_aligned_free((void *)ptr); }


/* memory pool */

/**
 * Block: a block of memory, keep many smaller chunks.
 *  
 *  alloc: return aligned memory:
 *      __PADDING__ (block pointer), (required size), (memory)
 *  free: remove the memory record from block. Return `this` pointer, if itself is empty.
 * */
struct Block {
    uint8_t     *mData;
    uint32_t     mSize;
    uint8_t      mCount;
    size_t       mCurPos;
    Block(uint32_t blockSize=32768)
        : mData(new uint8_t[blockSize]), mSize(blockSize)
        , mCount(0), mCurPos(0) {}
    ~Block() { delete[] mData; }

    static size_t actualSize(size_t size) { 
        static const int pointer_size  = sizeof(void *);
        static const int size_size     = sizeof(size_t);
        return size + (size_t)MEMORY_ALIGNMENT - 1 + pointer_size + size_size;
    }

    void *alloc(size_t _size) {
        static const uintptr_t pointer_size  = (uintptr_t)sizeof(void *);
        static const uintptr_t size_size     = (uintptr_t)sizeof(size_t);
        const uintptr_t alignment_1          = MEMORY_ALIGNMENT - 1;

        size_t actualSize = Block::actualSize(_size);
        if (mCurPos + actualSize > mSize) return nullptr;
#ifdef TEST_MEMORY
        printf("-> alloc 0x%X: start pos %d\n", this, mCurPos);
#endif
        uintptr_t start = (uintptr_t)mData + mCurPos + pointer_size + size_size;
        uintptr_t align = (start + alignment_1) & (~(alignment_1));

        // record block
        *((Block **)(align - pointer_size - size_size)) = (Block *)this;
        *(size_t *) (align - size_size) = _size;
        mCount += 1;
        mCurPos += actualSize;
        return (void *)align;
    }

    static Block * free(void *ptr) {
        static const uintptr_t pointer_size  = (uintptr_t)sizeof(void *);
        static const uintptr_t size_size     = (uintptr_t)sizeof(size_t);
        if (ptr == nullptr) return nullptr;
        Block *from = *((Block **) ( (uintptr_t)ptr - pointer_size - size_size ));
        if (--(from->mCount) == 0) {
            from->mCurPos = 0;
            return from;
        }
        else return nullptr;
    }

    static Block * queryFrom(void *ptr) {
        static const uintptr_t pointer_size  = (uintptr_t)sizeof(void *);
        static const uintptr_t size_size     = (uintptr_t)sizeof(size_t);
        if (ptr == nullptr) return nullptr;
        return *((Block **) ( (uintptr_t)ptr - pointer_size - size_size ));
    }

    static size_t querySize(void *ptr) {
        if (ptr == nullptr) return 0;
        static const uintptr_t size_size     = (uintptr_t)sizeof(size_t);
        return *(size_t *) ((uintptr_t)ptr - size_size);
    }
};

/**
 * MemoryArena: keep alloced blocks
 * 
 * */
class MemoryArena {
private:
    Block                  *mCurBlockPtr;
    uint32_t                mBlockSize;
    std::list<Block *>      mUsedBlocks;
    std::list<Block *>      mAvailableBlocks;

public:
    MemoryArena(uint32_t blockSize=32768)
        : mCurBlockPtr(nullptr)
        , mBlockSize(blockSize)
        , mUsedBlocks(0)
        , mAvailableBlocks(0) {}
    ~MemoryArena() {
        if (mCurBlockPtr) delete mCurBlockPtr;
        for (auto *block : mUsedBlocks)      delete block;
        for (auto *block : mAvailableBlocks) delete block;
    }

    void *alloc(uint32_t size) {
        void * ret = nullptr;
        if (!mCurBlockPtr || !(ret = mCurBlockPtr->alloc(size))) {
            // store in used
            if (mCurBlockPtr != nullptr) mUsedBlocks.push_back(mCurBlockPtr);
            mCurBlockPtr = nullptr;

            // find in available blocks
            for (auto iter = mAvailableBlocks.begin(); iter != mAvailableBlocks.end(); ++iter) {
                if ((ret = (*iter)->alloc(size)) != nullptr) {
                    mCurBlockPtr = (*iter);
                    mAvailableBlocks.erase(iter);
                    break;
                }
            }
            
            // still not found
            if (!mCurBlockPtr) {
                mCurBlockPtr = new Block(std::max(Block::actualSize(size), (size_t)mBlockSize));
            }
        }
        if (!ret) ret = mCurBlockPtr->alloc(size);
        return ret;
    }

    template <typename T>
    T *alloc(uint32_t count=1) {
        T *ret = (T*)this->alloc(count * sizeof(T));
        // for (uint32_t i = 0; i < count; ++i) new (&ret[i]) T();  // too slow
        ret = new (ret) T[count];  // faster
        return ret;
    }

    template <typename T>
    static size_t queryCount(T *ptr) {
        if (ptr == nullptr) return 0;
        return Block::querySize(ptr) / sizeof(T);
    }

    template <typename T>
    void free(T *ptr) {
        if (ptr == nullptr) return;
        const int count = Block::querySize(ptr) / sizeof(T);
        for (uint32_t i = 0; i < count; ++i) ptr[i].~T();
        Block * block = Block::free(ptr);
#ifdef TEST_MEMORY
        if (block) printf("-> reuse block 0x%X\n", block);
#endif
        if (block && block != mCurBlockPtr) {
            // check if the empty block is in used blocks
            bool find = false;
            for (auto iter = mUsedBlocks.begin(); iter != mUsedBlocks.end(); ++iter) {
                if ((*iter) == block) {
                    mUsedBlocks.erase(iter);
                    // recently used (just delete), push at first
                    mAvailableBlocks.push_front(block);
                    find = true;
                    break;
                }
            }
            if (!find) throw std::runtime_error("[MemoryArena]: empty block not found!\n");
        }
    }

    void reset() {
        mAvailableBlocks.splice(mAvailableBlocks.begin(), mUsedBlocks);
    }

};

}

#undef MEMORY_ALIGNMENT