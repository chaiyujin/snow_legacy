#pragma once
#include <list>
#include <stdlib.h>
#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <mutex>
#include "snow_log.h"

#define MEMORY_ALIGNMENT 32

namespace snow {

/* aligned memory allocate and free */

inline void *_aligned_malloc(size_t size, int alignment) {
    if (alignment & (alignment - 1)) {
        snow::fatal("[_aligned_malloc]: alignment should be 2 ^ n, rather than {0:d}", alignment);
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

template <typename T> inline T *  alignedMalloc(size_t number, int alignment=MEMORY_ALIGNMENT) {return (T*)snow::_aligned_malloc(sizeof(T) * number, alignment); }
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
    size_t       mSize;
    size_t       mCount;
    size_t       mCurPos;
    Block(size_t blockSize=32768)
        : mData(new uint8_t[blockSize]), mSize(blockSize)
        , mCount(0), mCurPos(0) {}
    ~Block() { delete[] mData; }

    void reset() { mCurPos = 0; mCount = 0; }

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
        snow::info("  alloc from block {:x}, start at {:d}", (intptr)this, mCurPos);
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
 * Using strategy of blob:
 * - each blob has size of 2^n (n >= 8)
 * - blobs are kept by two list: `avaliable`, `used` for each n.
 * - each blob only keep one item.
 * - alloc():
 *      1. first determine n.
 *      2. find avaliable blob in avaliable_n
 *      3. if no avaliable, than alloc new blob.
 *      4. move the blob into `used`
 * - free():
 *      1. first determine n.
 *      2. find the position in used.
 *      3. if not found, error
 *      4. move the blob into `avaliable`
 * - The arena should be effecient for repeatively allocating and freeing large memory, 
 *   but not suitable for small size due to it's overhead.
 * */
struct _NRecord {
    std::list<Block *> mFreeBlobs;
    std::list<Block *> mUsedBlobs;
    _NRecord() : mFreeBlobs(0), mUsedBlobs(0) {}
    ~_NRecord() {
        for (auto *ptr : mFreeBlobs) delete ptr;
        for (auto *ptr : mUsedBlobs) delete ptr;
        mFreeBlobs.clear();
        mUsedBlobs.clear();
    }
};

class MemoryArena {
private:
    std::mutex              mMutex;
    size_t                  mMinN, mMinBlobSize;
    size_t                  mMaxN, mMaxBlobSize;
    std::vector<_NRecord>   mNRecordList;
    std::vector<size_t>     mNTable;

    size_t getN(size_t size) {
        size_t n = std::max(mMinN, snow::CeilLog2(size));
        snow::assertion(n <= mMaxN, "size {} is too large", size);
        return n;
    }

public:
    MemoryArena(size_t minN = 8, size_t maxN = 32)
        : mMinN(minN), mMinBlobSize(0)
        , mMaxN(maxN), mMaxBlobSize(0)
        , mNRecordList(maxN + 1)
        , mNTable(maxN + 1) {
        size_t nSize = 1;
        for (size_t i = 0; i < mNTable.size(); ++i) {
            mNTable[i] = nSize; nSize *= 2;
        }
        mMinBlobSize = mNTable[mMinN];
        mMaxBlobSize = mNTable[mMaxN];
    }
    ~MemoryArena() {
        mNRecordList.clear();
        mNTable.clear();
    }

    void *alloc(size_t size) {
#ifdef TEST_MEMORY
        snow::info("[MemoryArena]: try to alloc {}", size);
#endif
        // for multi-threads
        std::lock_guard<std::mutex> lock(mMutex);
        // initialize the return
        void *ret = nullptr;
        // get actualsize and find the n
        size_t actualSize = Block::actualSize(size);
        size_t n = getN(actualSize);
        // try to alloc in n-record
        _NRecord &record = mNRecordList[n];
        Block *blob = nullptr;
        if (record.mFreeBlobs.size() > 0) {
            blob = record.mFreeBlobs.front();
            record.mFreeBlobs.pop_front();
        }
        else {
            blob = new Block(mNTable[n]);
        }
        record.mUsedBlobs.push_front(blob);
        ret = blob->alloc(size);
        return ret;
    }

    template <typename T>
    T *alloc(size_t count=1) {
        T *ret = (T*)this->alloc(count * sizeof(T));
        // for (size_t i = 0; i < count; ++i) new (&ret[i]) T();  // too slow
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
#ifdef TEST_MEMORY
        snow::info("[MemoryArena]: try to free {}", Block::querySize(ptr));
#endif
        if (ptr == nullptr) return;
        if (mNRecordList.size() == 0) return;
        std::lock_guard<std::mutex> lock(mMutex);
        // first call deconstructor
        const size_t freeSize = Block::querySize(ptr);
        size_t n = getN(Block::actualSize(freeSize));
        for (size_t i = 0; i < freeSize / sizeof(T); ++i) ptr[i].~T();
        // free the block
        Block *block = Block::free(ptr);
        // block should not be nullptr
        snow::assertion(block != nullptr);
        bool find = false;
        _NRecord &record = mNRecordList[n];
        for (auto iter = record.mUsedBlobs.begin(); iter!= record.mUsedBlobs.end(); ++iter) {
            if ((*iter) == block) {
                record.mFreeBlobs.push_front((*iter));
                record.mUsedBlobs.erase(iter);
                find = true;
                break;
            }
        }
        if (!find) snow::fatal("[MemoryArena]: empty block not found!\n");
#ifdef TEST_MEMORY
        snow::info("[MemoryArena] free() {:x} size {} pos {} avaliable {} used {}",
            (intptr_t)block, block->mSize, block->mCurPos,
            mAvailableBlocks.size(), mUsedBlocks.size());
#endif
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mMutex);
        for (size_t n = 0; n < mNRecordList.size(); ++n) {
            // clear used blobs
            for (auto *blob: mNRecordList[n].mUsedBlobs)
                blob->reset();
            // splice
            mNRecordList[n].mFreeBlobs.splice(mNRecordList[n].mFreeBlobs.begin(), mNRecordList[n].mUsedBlobs);
        }
    }

    void log(std::string tag="") {
        for (size_t n = mMinN; n < mMaxN; ++n) {
            snow::info("[{} blob {}] free {} used {}", tag, n, 
                mNRecordList[n].mFreeBlobs.size(),
                mNRecordList[n].mUsedBlobs.size());
        }
    }
};

}

#undef MEMORY_ALIGNMENT