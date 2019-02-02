#pragma once
#include "../common.h"

namespace snow { namespace math {

template <class Scalar>
struct _vec2 {
    union {
        Scalar d[2];
        struct {
            Scalar x, y;
        };
    }
};

}}