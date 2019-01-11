#pragma once
#include <stdio.h>
#include <stdlib.h>

#define SNOW_DEFAULT_ALIGNBYTES 32

// unused
#define SNOW_UNUSED(x) (void)x
// inline
#define SNOW_INLINE inline
// force inline
#if defined(__LINUX__) | defined(__linux__)
#define SNOW_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define SNOW_FORCE_INLINE __forceinline
#elif defined(__APPLE__)
#define SNOW_FORCE_INLINE inline __attribute__((always_inline))
#endif /* defined(__LINUX__) | defined(__linux__) */
// api
#ifdef WIN32
#ifdef SNOW_DLL_EXPORTS
#define SNOW_API __declspec(dllexport)
#else // !SNOW_DLL_EXPORTS
#define SNOW_API __declspec(dllimport)
#endif // SNOW_DLL_EXPORTS
#else // !WIN32
#define SNOW_API
#endif // WIN32

