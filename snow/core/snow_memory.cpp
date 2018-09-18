#include "snow_memory.h"

namespace snow {

void *_aligned_malloc(uint32_t size, int alignment) {
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

void _aligned_free(void *ptr) {
    if (ptr == nullptr) return;
    void *raw = *((void **) ( (uintptr_t)ptr - (uintptr_t)sizeof(void *) ));
    free(raw);
}

}