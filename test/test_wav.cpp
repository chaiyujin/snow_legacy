#include <snow.h>

int main() {
    snow::WavPCM pcm;
    pcm.read("../../assets/test.wav");

    return 0;
}