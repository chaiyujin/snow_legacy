/**
 * Pybind11 for anime viewer
 * */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "gui/window.h"

namespace py = pybind11;

std::vector<double> vector_from_numpy(py::array_t<double, py::array::c_style> &data) {
    assert(data.ndim() == 1);
    std::vector<double> ret(data.shape(0));
    memcpy(ret.data(), data.data(), sizeof(double) * data.shape(0));
    return ret;
}

S16Signal audio_from_numpy(py::array_t<int16_t, py::array::c_style> &data) {
    assert(data.ndim() == 1);
    std::vector<int16_t> ret(data.shape(0));
    memcpy(ret.data(), data.data(), sizeof(int16_t) * data.shape(0));
    return ret;
}

S16Signal audio_from_numpy(py::array_t<float, py::array::c_style> &data) {
    assert(data.ndim() == 1);
    std::vector<int16_t> ret(data.shape(0));
    for (size_t i = 0; i < data.shape(0); ++i) {
        ret[i] = data.data()[i] * 32767.0;
    }
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

std::vector<Vertices> anime_from_numpy(py::array_t<float, py::array::c_style> &data) {
    auto _to_frame = [](py::array_t<float, py::array::c_style> &anime, Vertices &frame, int r) -> void {
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

void new_app(std::string type) {
    if (type == "obj")
        Application::newAPP(ModelType::Obj);
    else if (type == "bilinear")
        Application::newAPP(ModelType::Bilinear);
    else
        snow::fatal("[anime_viewer]: new_app(type), type should be `obj` or `bilinear`");
}

void run(double fps) {
    Application::run(fps);
}

void offscreen(std::string videoPath, double fps, int videoWidth, int videoHeight) {
    Application::offscreen(videoPath, fps, videoWidth, videoHeight);
}

/**
 * =========================================
 *              window private
 * =========================================
 **/

void set_text(std::string window, std::string text) {
    Application::setText(window, text);
}

void add_audio_s16(std::string tag, py::array_t<int16_t, py::array::c_style> &audio, int samplerate) {
    auto data = audio_from_numpy(audio);
    Application::addAudio(tag, data, samplerate);
}

void add_audio_f32(std::string tag, py::array_t<float, py::array::c_style> &audio, int samplerate) {
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

/* for ModelType::Obj */

void set_obj(std::string window, std::string filename) {
    Application::setObj(window, filename);
}

void set_anime(std::string window, py::array_t<float, py::array::c_style> &anime) {
    std::vector<Vertices> frames = anime_from_numpy(anime);
    Application::setAnime(window, frames);
}

/* for ModelType::Bilinear */
void set_iden(std::string window, py::array_t<double, py::array::c_style> &iden) {
    std::vector<double> _iden = vector_from_numpy(iden);
    Application::setIden(window, _iden);
}

void set_expr_list(std::string window, py::array_t<double, py::array::c_style> &exprList) {
    assert(exprList.ndim() == 2);
    std::vector<std::vector<double>> _exprList;
    for (size_t i = 0; i < exprList.shape(0); ++i) {
        std::vector<double> expr(exprList.shape(1));
        memcpy(expr.data(), exprList.data(i), sizeof(double) * expr.size());
        _exprList.push_back(expr);
    }
    Application::setExprList(window, _exprList);
}

void initialize_bilinear(std::string root_dir) {
    FaceDB::Initialize(root_dir);
}

PYBIND11_MODULE(anime_viewer11, m) {
    m.doc() = "pybind11 AnimeViewer plugin"; // optional module docstring

    m.def("initialize_bilinear",&initialize_bilinear,"");
    m.def("new_app",            &new_app,           "create a new app for `obj` or `bilinear`");
    m.def("set_text",           &set_text,          "set the text for certain window");
    m.def("add_audio_s16",      &add_audio_s16,     "add audio for entire app");
    m.def("add_audio_f32",      &add_audio_f32,     "add audio for entire app");
    m.def("add_scroll_image",   &add_scroll_image,  "");
    // for ModelType::Obj
    m.def("set_obj",            &set_obj,           "set obj for certain window");
    m.def("set_anime",          &set_anime,         "set anime for certain window");
    // for ModelType::Bilinear
    m.def("set_iden",           &set_iden,          "set iden for certain window");
    m.def("set_expr_list",      &set_expr_list,     "set expr list for certain window");
    // run or terminate
    m.def("run",                &run,               "run app with given fps");
    m.def("offscreen",          &offscreen,         "offscreen rendering");
    m.def("terminate",          &terminate,         "terminate app");
}
