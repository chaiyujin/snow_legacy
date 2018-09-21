// http://pfultz2.com/blog/2014/09/05/pipable-functions/
#pragma once

#include <functional>

namespace snow::libfp {

struct FuncWrapper {

};

template <class F>
struct PipeClosure : public F {
    template <class ...Xs>
    PipeClosure(Xs && ...xs) : F(std::forward<Xs>(xs)...) {}
};

template <class T, class F>
decltype(auto) operator|(T &&x, const PipeClosure<F> &p) {
    return p(std::forward<T>(x));
}

}