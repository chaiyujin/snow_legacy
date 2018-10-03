#include <snow.h>
using namespace snow;

int main() {
    Image image = Image::Read("../../assets/tsuki.jpeg");
    Image::Write("../../assets/test.png", image);
    return 0;
}