#include "arena.h"
#include "allocator.h"

namespace snow { namespace memory {

/* aligned malloc */

void *_snow_alloc(size_t size, size_t align) {
#ifdef WIN32
    return _aligned_malloc(size, align);
#else
    void *ptr = nullptr;
    posix_memalign(&ptr, align, size);
    return ptr;
#endif
}

void _snow_free(void *ptr) {
#ifdef WIN32
    _aligned_free(ptr);
#else
    free((void*)ptr);
#endif
}

}}
