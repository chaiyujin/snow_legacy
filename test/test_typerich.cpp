#include <iostream>
#include "../libsnow/core/utils/timer.h"
using namespace snow;

int main() {
    {
        double x;
        snow::TimerGuard timeGuard("this");
        for (int j = 0; j < 10000; ++j)
            for (int i =0 ; i < 1000000; ++i)
                x = std::pow(i, 3);
    }
    return 0;
}
