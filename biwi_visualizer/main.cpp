#include <snow.h>
#include <string>
#include "window.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <thread>

snow::App *app = nullptr;
std::map<std::string, std::string> objMap;

void run(std::vector<std::pair<std::string, std::string>> win_obj_pairs) {
    app = new App();
    auto t = std::thread([&]() -> void {
        for (auto pair : win_obj_pairs) {
            auto *win = new ObjWindow(pair.first.c_str());
            win->loadObj(pair.second.c_str());
            app->addWindow(win);
        }
        app->run();
        delete app;
        app = nullptr;
    });
    t.join();
}

PYBIND11_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("run", &run, "");
}
