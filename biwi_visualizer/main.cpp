#include <snow.h>
#include <string>
#include "window.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <thread>

namespace py = pybind11;

snow::App *app = nullptr;
std::vector<std::pair<std::string, std::string>> gWinObjPairs;
std::map<std::string, std::vector<Frame>> gWinAnimeMap;
int gCurIndex = -1;

void _toFrame(py::array_t<float> &anime, Frame &frame, int r) {
    // anime
    int C = anime.shape(1);
    for (int i = 0; i * 3 < C; ++i) {
        float x = *anime.data(r, i * 3);
        float y = *anime.data(r, i * 3 + 1);
        float z = *anime.data(r, i * 3 + 2);
        frame.emplace_back(x, y, z);
    }
}

void _toAnime(py::array_t<float> &anime, std::vector<Frame> &frames) {
    frames.clear();
    for (int r = 0; r < anime.shape(0); ++r) {
        frames.emplace_back();
        auto &frame = frames[frames.size() - 1];
        _toFrame(anime, frame, r);
    }
}

void terminate() {
    gWinObjPairs.clear();
    gWinAnimeMap.clear();
    gCurIndex = -1;
}

void begin_setting() {
    terminate();
}

void add_obj(std::string winName, std::string objPath) {
    gWinObjPairs.push_back(std::pair<std::string, std::string>(winName, objPath));
}

void add_anime(std::string winName, py::array_t<float> &anime) {
    std::vector<Frame> frames;
    _toAnime(anime, frames);
    gWinAnimeMap.insert(std::pair<std::string, std::vector<Frame>>(winName, frames));
    gCurIndex = 0;
}

void run() {
    app = new App();
    auto t = std::thread([&]() -> void {
        for (auto pair : gWinObjPairs) {
            auto *win = new ObjWindow(pair.first.c_str());
            auto it = gWinAnimeMap.find(pair.first);
            if (it != gWinAnimeMap.end()) {
                win->setAnime(&gCurIndex, &(it->second));
            }
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

    m.def("begin_setting", &begin_setting, "");
    m.def("add_obj", &add_obj, "");
    m.def("add_anime", &add_anime, "");
    m.def("run", &run, "");
    m.def("terminate", &terminate, "");
}
