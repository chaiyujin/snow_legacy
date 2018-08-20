#include <snow.h>
#include <string>
#include "window.h"
#include <pybind11/pybind11.h>
#include <thread>

snow::App *app = nullptr;

void run(std::string obj_path) {
    bool stop = false;
    app = new App();
    auto t = std::thread([&]() -> void {
        auto *win = new ObjWindow("obj viewer");
        app->addWindow(win);
        app->run();
        printf("quit\n");
        delete app;
        app = nullptr;
    });
    t.detach();
    while (!stop) {
        auto *p = app->getWindow("obj viewer");
        if (p) {
            ((ObjWindow*)p)->loadObj(obj_path);
            break;
        }
    }
}

void stop() {
    if (app) {
        auto *p = app->getWindow("obj viewer");
        if (p) {
            app->quit();
        }
    }
}

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("run", &run, "");
    m.def("stop", &stop, "");
    m.def("add", &add, "");
}