#include "window.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please give a .obj filepath.\n");
        return 0;
    }
    Application::newAPP();
    Application::setObj("window0", argv[1]);
    Application::run(25.0);
    Application::terminate();

    return 0;
}