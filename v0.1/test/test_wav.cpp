#include <snow.h>

int main() {
    snow::WavPCM pcm_read, pcm_write;
    pcm_read.read("../../assets/test.wav");

    pcm_write.setSampleRate(pcm_read.sampleRate());
    pcm_write.addChannel(pcm_read.channel(0));
    pcm_write.write("../../assets/test_write.wav");
    return 0;
}
