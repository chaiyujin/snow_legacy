#define SNOW_MODULE_LIBFP
#include <snow.h>
using namespace snow::libfp;

struct add_one_f {
    template <class T>
    auto operator()(T x) const {
        return x + 1;
    }
};

std::function<int(int)> add_one_func = [](int x) -> int {
    return x + 1;
};

int main() {

    const PipeClosure<add_one_f> add_one;
    int number3 = 1 | add_one | add_one;
    std::cout << number3 << std::endl;

    return 0;
}