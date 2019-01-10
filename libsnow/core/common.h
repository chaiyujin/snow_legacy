#pragma once
#include <stdio.h>
#include <stdlib.h>

#define SNOW_INLINE inline
/* macro forcing a function to be inlined */
#if defined(__LINUX__) | defined(__linux__)
#define SNOW_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define SNOW_FORCE_INLINE __forceinline
#endif /* defined(__LINUX__) | defined(__linux__) */

#define SNOW_UNUSED(x) (void)x
