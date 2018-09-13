#include <snow.h>
#include <iostream>
#include "video/reader.h"
#include "window/player.h"
using namespace snow;

void write_image(const char *name, const VideoFrame &input_frame) {
    FILE *fp = fopen(name, "w");
    VideoFrame frame;
    if (input_frame.mIsDepth) DepthVideoReader::colorize(input_frame, frame);
    else frame = input_frame;
    fprintf(fp, "P3\n%d %d\n255\n", frame.mWidth, frame.mHeight);
    for (int i = 0; i < frame.mWidth * frame.mHeight; ++i) {
        for (int j = 0; j < 3; ++j)
            fprintf(fp, "%d ", frame.mData.get()[i * 4 + j]);
    }
    fclose(fp);
}

int main() {
    snow::App app;
    PlayerWindow * player = new PlayerWindow();
    player->openVideo("../../assets/test.mkv");
    app.addWindow(player);
    app.run();

    return 0;
}