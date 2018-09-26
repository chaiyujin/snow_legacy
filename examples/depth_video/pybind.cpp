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
    collectVideo(filepath, frames, audio);

    py::array_t<float> np_audio(audio.size());
    py::buffer_info buf = np_audio.request();
    memcpy(buf.ptr, audio.data(), sizeof(float) * audio.size());

    return py::dict("audio"_a=np_audio, "sample_rate"_a=SampleRate);
}

PYBIND11_MODULE(depth11, m) {
    m.doc() = "pybind11 depth video plugin"; // optional module docstring

    m.def("set_sample_rate",    &set_sample_rate,   "");
    m.def("collect_video",      &collect_video,     "");
}
