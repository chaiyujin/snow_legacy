#include <snow/snow.h>
#include <memory>

class A : public snow::memory::Allocator<16> {
public:
    int val;
    A(int v=10) : val(v) {}
};

template <class ost> ost&operator<<(ost&out, const A& a) {
    out << a.val;
    return out;
}

int main() {
    {
        snow::Image image(320, 240, 3);
        image.resize(1024, 720, 3);
    }
    // {
    //     auto a = std::shared_ptr<A>(new A(100));
    //     snow::log::info("Val of A is {}", *a);
    // }
    return 0;
}
