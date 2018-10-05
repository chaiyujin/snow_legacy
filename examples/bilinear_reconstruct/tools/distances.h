#pragma once
#include <cmath>
#include <algorithm>
#include <snow.h>

struct PointToLine2D {
    template <typename T>
    static T sqrDistance(T x, T y, T x0, T y0, T x1, T y1) {
        T A = y1 - y0;
        T B = x0 - x1;
        T C = x1 * y0 - x0 * y1;
        T f = (A * x + B * y + C);
        return (f * f) / (A * A + B * B);
    }
    template <typename T>
    static snow::_float2<T> diffSqrDistance(T x, T y, T x0, T y0, T x1, T y1) {
        T A = y1 - y0;
        T B = x0 - x1;
        T C = x1 * y0 - x0 * y1;
        T f = (A * x + B * y + C);
        T Factor = (T)2 * f / (A * A + B * B);
        return { Factor * A, Factor * B };
    }
    
};
