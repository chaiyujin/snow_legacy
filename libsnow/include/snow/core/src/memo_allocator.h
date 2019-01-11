#pragma once
#include "common.h"
#include "tool_log.h"
#include <vector>
#include <memory>
namespace snow { namespace memory {

template <class T, int AlignBytes=SNOW_DEFAULT_ALIGNBYTES>
inline T *allocate(size_t size) {
    void *ptr = nullptr;
    // posix_memalign(&ptr, AlignBytes, size);
    ptr = malloc(size);
    log::debug("allocate({}) -> {}", size, ptr);
    return (T*)ptr;
}

inline void deallocate(void *ptr) {
    if (ptr != nullptr) {
        log::debug("deallocate({})", (void*)ptr);
        free((void*)ptr);
    }
}

/** An allocator as base class for 
 *  faster memory allocation.
 * */
template <int AlignBytes=SNOW_DEFAULT_ALIGNBYTES>
class Allocator {

public:
    SNOW_FORCE_INLINE void * operator new(size_t size) {
        log::debug("Allocator::new");
        return allocate<void, AlignBytes>(size);
    }
    SNOW_FORCE_INLINE void operator delete(void * ptr) {
        log::debug("Allocator::delete");
        deallocate(ptr);
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size) {
        log::debug("Allocator::new[]");
        return allocate<void, AlignBytes>(size);
    }
    SNOW_FORCE_INLINE void operator delete[](void * ptr) {
        log::debug("Allocator::delete[]");
        deallocate(ptr);
    }
    /* placement new */
    SNOW_FORCE_INLINE void * operator new  (size_t size, void *ptr) {
        log::debug("Allocator::placement new");
        SNOW_UNUSED(size); return ptr;
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size, void *ptr) {
        log::debug("Allocator::placement new[]");
        SNOW_UNUSED(size); return ptr;
    }
};

}}
