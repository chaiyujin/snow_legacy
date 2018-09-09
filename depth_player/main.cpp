#include <snow.h>
#include <iostream>
#include "video/reader.h"

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

    int3 a = {1, 2};
    float3 b = {10, 10};

    std::cout << a << std::endl
              << b << std::endl
              << dot(b, b) << std::endl
              << dot(float3{1, 2, 3}, float3{3, 2, 1}) << std::endl
              << cross(float3{11.23, 2.14, 3.54}, float3{3, 231, 1}) << std::endl
              << dot(float2{1, 10}, float2{4, -1}) << std::endl
              << cross(float2{21.12, 3.14}, float2{5.13, 1.3}) << std::endl
              << euclidean_norm(float2{4, 3}) << std::endl;

    DepthVideoReader::initialize_ffmpeg();
    DepthVideoReader reader("/home/chaiyujin/Desktop/0-0-0.mkv");
    if (reader.open()) {
        int frameCount = 0;
        do {
            auto framepair = reader.read_frame_pair();
            if (framepair.first.isNull()) break;
            std::cout << ++frameCount << "\n";
            write_image("color.ppm", framepair.first);
            write_image("depth.ppm", framepair.second);
            break;
        } while (true);
        std::cout << "\n";
    }

    return 0;
}