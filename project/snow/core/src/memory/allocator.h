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

template <class T>
SNOW_INLINE T *allocate(size_t count, size_t alignBytes=SNOW_DEFAULT_ALIGNBYTES) {
    const size_t size = count * sizeof(T);
    void *ptr = _snow_alloc(size, alignBytes);
    log::debug("allocate({}) -> {}", size, ptr);
    return (T*)ptr;
}

template <>
SNOW_INLINE void *allocate<void>(size_t count, size_t alignBytes) {
    void *ptr = _snow_alloc(count, alignBytes);
    log::debug("allocate({}) -> {}", count, ptr);
    return ptr;
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
        return allocate<void>(size, AlignBytes);
    }
    SNOW_FORCE_INLINE void operator delete(void * ptr) {
        log::debug("allocator::delete");
        deallocate(ptr);
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size) {
        log::debug("allocator::new[]");
        return allocate<void>(size, AlignBytes);
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
