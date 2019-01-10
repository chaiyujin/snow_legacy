#pragma once
#include "../common.h"

namespace snow {

void debug(const char *msg) { printf("%s\n", msg); }

/** An allocator as base class for 
 *  faster memory allocation.
 * */
template <int AlignBytes>
class Allocator {

public:
    SNOW_FORCE_INLINE void * operator new(size_t size) {
        debug("new");
        void* ptr = nullptr;
        posix_memalign(&ptr, AlignBytes, size);
        return ptr;
    }
    SNOW_FORCE_INLINE void operator delete(void * ptr) {
        debug("delete");
        free(ptr);
    }
    SNOW_FORCE_INLINE void * operator new[](size_t size) {
        debug("new[]");
        void* ptr = nullptr;
        posix_memalign(&ptr, AlignBytes, size);
        return ptr;
    }
    SNOW_FORCE_INLINE void operator delete[](void * ptr) {
        debug("delete[]");
        free(ptr);
    }
    /* placement new */
    SNOW_FORCE_INLINE void * operator new  (size_t size, void *ptr) { SNOW_UNUSED(size); return ptr; }
    SNOW_FORCE_INLINE void * operator new[](size_t size, void *ptr) { SNOW_UNUSED(size); return ptr; }
};

}
