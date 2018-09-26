#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "process.h"

namespace py = pybind11;
using namespace pybind11::literals;

void set_sample_rate(int sr) {
    setSampleRate(sr);
}

py::dict collect_video(const std::string &filepath) {
    std::vector<ModelFrame> frames;
    std::vector<float>      audio;
    double fps = collectVideo(filepath, frames, audio);

    if (fps < 0.0 || frames.size() == 0 || audio.size() == 0) return py::none();

    double scale = frames[0].mScale;
    py::array_t<float> np_audio(audio.size());
    py::array_t<double> np_iden(frames[0].mIden.size());
    py::array_t<double> np_exprs({frames.size(), frames[0].mExpr.size()});
   
    /* copy audio */
    memcpy(np_audio.mutable_data(), audio.data(), sizeof(float) * audio.size());
    
    /* iden */
    py::buffer_info buf = np_iden.request();
    memcpy(np_iden.mutable_data(), frames[0].mIden.data(), sizeof(double) * frames[0].mIden.size());
    
    /* exprs */ for (int r = 0; r < (int)frames.size(); ++r) {
        memcpy(np_exprs.mutable_data(r), frames[r].mExpr.data(), sizeof(double) * frames[r].mExpr.size());
    }

    return py::dict("audio"_a=np_audio,
                    "sample_rate"_a=SampleRate,
                    "fps"_a=fps,
                    "scale"_a=scale,
                    "iden"_a=np_iden,
                    "expr_frames"_a=np_exprs);
}

PYBIND11_MODULE(depth11, m) {
    m.doc() = "pybind11 depth video plugin"; // optional module docstring

    m.def("set_sample_rate",    &set_sample_rate,   "");
    m.def("collect_video",      &collect_video,     "");
}
