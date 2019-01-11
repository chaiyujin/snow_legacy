#pragma once
#include "../common.h"
#include "../tools/log.h"
#include "arena.h"
#include <vector>
#include <memory>
namespace snow { namespace memory {

/* api of allocation */
SNOW_API void *_snow_alloc(size_t size, size_t align);
SNOW_API void _snow_free(void *ptr);

template <class T, int AlignBytes=SNOW_DEFAULT_ALIGNBYTES>
SNOW_INLINE T *allocate(size_t size) {
    void *ptr = _snow_alloc(size, AlignBytes);
    log::debug("allocate({}) -> {}", size, ptr);
    return (T*)ptr;
}

SNOW_INLINE void deallocate(void *ptr) {
    if (ptr != nullptr) {
        log::debug("deallocate({})", (void*)ptr);
        _snow_free(ptr);
    }
}

/** An allocator as base class for 
 *  faster memory allocation.
 * */
template <int AlignBytes=SNOW_DEFAULT_ALIGNBYTES>
class Allocator {

public:
    SNOW_FORCE_INLINE void * operator new(size_t size) {
        log::debug("allocator::new");
        return allocate<void, AlignBytes>(size);
    }
    SNOW_FORCE_INLINE void operator delete(void * ptr) {
        log::debug("allocator::delete");
        deallocate(ptr);
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size) {
        log::debug("allocator::new[]");
        return allocate<void, AlignBytes>(size);
    }
    SNOW_FORCE_INLINE void operator delete[](void * ptr) {
        log::debug("allocator::delete[]");
        deallocate(ptr);
    }
    /* placement new */
    SNOW_FORCE_INLINE void * operator new  (size_t size, void *ptr) {
        log::debug("allocator::placement new");
        SNOW_UNUSED(size); return ptr;
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size, void *ptr) {
        log::debug("allocator::placement new[]");
        SNOW_UNUSED(size); return ptr;
    }
};

}}
