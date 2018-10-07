#pragma once
#include <cmath>
#include <algorithm>
#include <snow.h>

/*********************
 *      distance     *
 *********************/
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
    template <typename T>
    static T distance(T x, T y, T x0, T y0, T x1, T y1) {
        return std::sqrt(PointToLine2D::sqrDistance<T>(x, y, x0, y0, x1, y1));
    }
    template <typename T>
    static snow::_float2<T> diffDistance(T x, T y, T x0, T y0, T x1, T y1) {
        T Factor = (T)0.5 / PointToLine2D::distance<T>(x, y, x0, y0, x1, y1);
        return PointToLine2D::diffSqrDistance(x, y, x0, y0, x1, y1) * Factor;
    }
};

struct Point2Point2D {
    template <typename T>
    static T sqrDistance(T x, T y, T x0, T y0) {
        T A = (x - x0);
        T B = (y - y0);
        return A * A + B * B;
    }
    template <typename T>
    static snow::_float2<T> diffSqrDistance(T x, T y, T x0, T y0) {
        T A = (x - x0);
        T B = (y - y0);
        return { (T)2 * A, (T)2 * B };
    }
    template <typename T>
    static T distance(T x, T y, T x0, T y0) {
        T A = (x - x0);
        T B = (y - y0);
        return std::sqrt(A * A + B * B);
    }
    template <typename T>
    static snow::_float2<T> diffDistance(T x, T y, T x0, T y0) {
        T Factor = (T)0.5 / Point2Point2D::distance<T>(x, y, x0, y0);
        return Point2Point2D::diffSqrDistance(x, y, x0, y0) * Factor;
    }
};


/*********************
 *         Mat       *
 *********************/
namespace operation {
    struct Transform {
        template<typename T, glm::qualifier Q>
        static glm::vec<4, T, Q> forward (const glm::mat<4, 4, T, Q> &m, const glm::vec<4, T, Q> &input) {
            return m * input;
        }
        template<typename T, glm::qualifier Q>
        static glm::vec<4, T, Q> backward(const glm::mat<4, 4, T, Q> &m, const glm::vec<4, T, Q> &gradIn) {
            return glm::transpose(m) * gradIn;
        }
    };
    
    struct Project {
        template <typename T, glm::qualifier Q>
        static glm::vec<4, T, Q> forward (const glm::vec<4, T, Q> &input) {
            return input / input.w;
        }
        template <typename T, glm::qualifier Q>
        static glm::vec<4, T, Q> backward(const glm::vec<4, T, Q> &input, const glm::vec<4, T, Q> &gradIn) {
            T iw = (T)1 / input.w;
            T iw2 = iw * iw;
            return {
                gradIn.x * iw, gradIn.y * iw, gradIn.z * iw,
                -iw2 * (gradIn.x * input.x + gradIn.y * input.y + gradIn.z * input.z)
            };
        }
    };
}
