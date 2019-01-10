#include <iostream>
#include <core/core.h>
using namespace snow;

class A: public Allocator<16> {
public:
    double a;
    A(): a(10) {}
};

int main() {
    #ifdef NDEBUG
    exit(0);
    #endif
    {
        auto *a = new A();
        void *mem = malloc(sizeof(A));
        new (mem) A();
        std::cout << "val new: " << a->a << std::endl;
        std::cout << "val plc: " << ((A*)mem)->a << std::endl;
        delete a;
        free(mem);
    }
    {
        auto *a = new A[100]();
        for (int i = 0; i < 100; ++i)
            if (!(a[i].a == 10))
                log::fatal("(new)cao ni ma");
        delete[] a;
        void *b = malloc(sizeof(A)*100);
        // new (b) A[100]();
        for (int i = 0; i < 100; ++i)
            log::assertion(((A*)b)[i].a == 10);
            // if (!(((A*)b)[i].a == 10))
            //     log::fatal("(plc)cao ni ma");
        free(b);
    }
    return 0;
}
