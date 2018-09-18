#pragma once

#include <x86intrin.h>

namespace snow {

#ifdef SNOW_AVX
inline double dot(const double *a, const double *b, size_t len) {
    const size_t nBlockWidth = 16;
    const size_t cntBlock    = len / nBlockWidth;
    const size_t cntRem      = len % nBlockWidth;

    __m256d avxSum = _mm256_setzero_pd();
    __m256d avxLoadA0, avxLoadB0;
    __m256d avxLoadA1, avxLoadB1;
    __m256d avxLoadA2, avxLoadB2;
    __m256d avxLoadA3, avxLoadB3;

    double result = 0.0;
    size_t i;

    bool alignA = ((intptr_t)a & 31) == 0;
    bool alignB = ((intptr_t)b & 31) == 0;

    for (i = 0; i < cntBlock; ++i) {
        if (alignA && alignB) {
            avxLoadA0 = _mm256_load_pd(a);
            avxLoadA1 = _mm256_load_pd(a+4);
            avxLoadA2 = _mm256_load_pd(a+8);
            avxLoadA3 = _mm256_load_pd(a+12);
            avxLoadB0 = _mm256_load_pd(b);
            avxLoadB1 = _mm256_load_pd(b+4);
            avxLoadB2 = _mm256_load_pd(b+8);
            avxLoadB3 = _mm256_load_pd(b+12);
        }
        else {
            avxLoadA0 = _mm256_loadu_pd(a);
            avxLoadA1 = _mm256_loadu_pd(a+4);
            avxLoadA2 = _mm256_loadu_pd(a+8);
            avxLoadA3 = _mm256_loadu_pd(a+12);
            avxLoadB0 = _mm256_loadu_pd(b);
            avxLoadB1 = _mm256_loadu_pd(b+4);
            avxLoadB2 = _mm256_loadu_pd(b+8);
            avxLoadB3 = _mm256_loadu_pd(b+12);
        }

        __m256d xy0 = _mm256_mul_pd( avxLoadA0, avxLoadB0 );
        __m256d xy1 = _mm256_mul_pd( avxLoadA1, avxLoadB1 );
        __m256d xy2 = _mm256_mul_pd( avxLoadA2, avxLoadB2 );
        __m256d xy3 = _mm256_mul_pd( avxLoadA3, avxLoadB3 );

        __m256d temp01 = _mm256_hadd_pd( xy0, xy1 );
        __m256d temp23 = _mm256_hadd_pd( xy2, xy3 );
        __m256d swapped = _mm256_permute2f128_pd( temp01, temp23, 0x21 );
        __m256d blended = _mm256_blend_pd(temp01, temp23, 0b1100);

        __m256d dotproduct = _mm256_add_pd( swapped, blended );

        avxSum = _mm256_add_pd(avxSum, dotproduct);
        
        a += nBlockWidth;
        b += nBlockWidth;
    }
    const double *q = (const double*)&avxSum;
    result = q[0] + q[1] + q[2] + q[3];

    for (i = 0; i < cntRem; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

inline void addwb(double *a, const double *b, double w, int len) {

    const size_t nBlockWidth = 8;
    const size_t cntRem      = len & (nBlockWidth - 1);
    const size_t cntBlock    = len >> 3;
    size_t i;
    __m256d avxSum0, avxSum1;
    __m256d avxLoadA0, avxLoadB0;
    __m256d avxLoadA1, avxLoadB1;
    const __m256d avxWeight = _mm256_set1_pd(w);

    bool alignA = ((intptr_t)a & 31) == 0;
    bool alignB = ((intptr_t)b & 31) == 0;
    for (i = 0; i < cntBlock; ++i) {
        if (alignA && alignB) {
            avxLoadA0 = _mm256_load_pd(a);
            avxLoadA1 = _mm256_load_pd(a + 4);
            avxLoadB0 = _mm256_load_pd(b);
            avxLoadB1 = _mm256_load_pd(b + 4);
        }
        else {
            avxLoadA0 = _mm256_loadu_pd(a);
            avxLoadA1 = _mm256_loadu_pd(a + 4);
            avxLoadB0 = _mm256_loadu_pd(b);
            avxLoadB1 = _mm256_loadu_pd(b + 4);
        }
        avxSum0  = _mm256_add_pd(avxLoadA0, _mm256_mul_pd(avxLoadB0, avxWeight));
        avxSum1  = _mm256_add_pd(avxLoadA1, _mm256_mul_pd(avxLoadB1, avxWeight));
        if (alignA) {
            _mm256_store_pd(a, avxSum0);
            _mm256_store_pd(a + 4, avxSum1);
        }
        else {
            _mm256_storeu_pd(a, avxSum0);
            _mm256_storeu_pd(a + 4, avxSum1);
        }
        a += nBlockWidth;
        b += nBlockWidth;
    }

    for (i = 0; i < cntRem; ++i) {
        a[i] += b[i] * w;
    }

}

/* for test */

inline double dot_normally(const double *a, const double *b, size_t len) {
    double result = 0.0;
    for (size_t i = 0; i < len; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

inline void addwb_normally(double *a, const double *b, double w, int len)  {
    for (int i = 0; i < len; ++i) {
        a[i] += b[i] * w;
    }
}

#else

inline double dot(const double *a, const double *b, size_t len) {
    double result = 0.0;
    for (size_t i = 0; i < len; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

inline void addwb(double *a, const double *b, double w, int len)  {
    for (int i = 0; i < len; ++i) {
        a[i] += b[i] * w;
    }
}

#endif

}