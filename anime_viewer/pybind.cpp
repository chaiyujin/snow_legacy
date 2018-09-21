#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "gui/window.h"

namespace py = pybind11;

S16Signal audio_from_numpy(py::array_t<int16_t> &data) {
    assert(data.ndim() == 1);
    std::vector<int16_t> ret(data.shape(0));
    memcpy(ret.data(), data.data(), sizeof(int16_t) * data.shape(0));
    return ret;
}

ScrollImage scroll_image_from_numpy(py::array_t<uint8_t, py::array::c_style> &data, int win_len, int hop_len) {
    assert(data.ndim() == 3 && data.shape(2) == 3); // rbg
    size_t size = data.shape(0) * data.shape(1) * data.shape(2);
    ScrollImage ret;
    ret.mImg.resize(size);
    ret.mRows = data.shape(0);
    ret.mCols = data.shape(1);
    ret.mWinLength = win_len;
    ret.mHopLength = hop_len;
    memcpy(ret.mImg.data(), data.data(), size);
    return ret;
}

std::vector<Vertices> anime_from_numpy(py::array_t<float> &data) {
    auto _to_frame = [](py::array_t<float> &anime, Vertices &frame, int r) -> void {
        // anime
        int C = anime.shape(1);
        for (int i = 0; i * 3 < C; ++i) {
            float x = *anime.data(r, i * 3);
            float y = *anime.data(r, i * 3 + 1);
            float z = *anime.data(r, i * 3 + 2);
            frame.emplace_back(x, y, z);
        }
    };

    std::vector<Vertices> frames;
    for (int r = 0; r < data.shape(0); ++r) {
        frames.emplace_back();
        auto &frame = frames[frames.size() - 1];
        _to_frame(data, frame, r);
    }
    return frames;
}

void terminate() {
    Application::terminate();
}

void new_app() {
    Application::newAPP();
}

void run(double fps) {
    Application::run(fps);
}

void set_text(std::string window, std::string text) {
    Application::setText(window, text);
}

/* window private */

void set_obj(std::string window, std::string filename) {
    Application::setObj(window, filename);
}

void set_anime(std::string window, py::array_t<float> &anime) {
    std::vector<Vertices> frames = anime_from_numpy(anime);
    Application::setAnime(window, frames);
}

void add_audio(std::string tag, py::array_t<int16_t> &audio, int samplerate) {
    auto data = audio_from_numpy(audio);
    Application::addAudio(tag, data, samplerate);
}

void add_scroll_image(std::string window, std::string title,
                      py::array_t<uint8_t, py::array::c_style> img,
                      int win_length,
                      int hop_length) {
    assert(img.shape(2) == 3);
    // img.transpoe(1, 0, 2);
    ScrollImage scroll_image = scroll_image_from_numpy(img, win_length, hop_length);
    Application::addScrollImage(window, title, scroll_image);
}

PYBIND11_MODULE(AnimeViewer, m) {
    m.doc() = "pybind11 AnimeViewer plugin"; // optional module docstring

    m.def("new_app",            &new_app, "");
    m.def("set_text",           &set_text, "");
    m.def("add_audio",          &add_audio, "");
    m.def("set_obj",            &set_obj, "");
    m.def("set_anime",          &set_anime, "");
    m.def("add_scroll_image",   &add_scroll_image, "");
    m.def("run",                &run, "");
    m.def("terminate",          &terminate, "");
}
