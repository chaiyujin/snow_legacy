#include "process.h"

int main() {
    std::vector<ModelFrame> modelFrames;
    std::vector<float> audioTrack;
    collectVideo("../../../assets/test_depth/0-0-1.mkv",
                 modelFrames, audioTrack);
    return 0;
}
