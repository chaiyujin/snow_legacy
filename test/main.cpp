#include <snow/snow.h>
#include <memory>

class A : public snow::memory::Allocator<> {
public:
    int val;
    A(int v=10) : val(v) {}
};

template <class ost> ost&operator<<(ost&out, const A& a) {
    out << a.val;
    return out;
}

int main() {
    {
        snow::WavPcm wav;
        wav.load("../test.wav");
        wav.dumpHeader();
        wav.save("test.wav");
    }
    {
        snow::log::info("snow version: {}", snow::__version__());
        auto image = snow::Image::Load("../scene.jpeg");
        auto flipped = image.flippedX().flipY();
        image = snow::Image::MergeY(image, flipped);
        image.save("test.png");
    }
    {
        auto a = std::shared_ptr<A>(new A(100));
        snow::log::info("Val of A is {}", *a);
    }
    return 0;
}
