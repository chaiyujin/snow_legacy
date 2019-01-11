#include <snow/snow.h>

class A {
public:
    int val;
    A(int v=10) : val(v) {}
};

template <class ost> ost&operator<<(ost&out, const A& a) {
    out << a.val;
    return out;
}

int main() {
    A a;
    snow::log::info("Val of A is {}", a);
    return 0;
}
