#include "process.h"

int main() {
    ModelShared modelShared;
    std::vector<ModelFrame> modelFrames;
    std::vector<float> audioTrack;
    collectVideo("../../../assets/test_depth/0-0-1.mkv",
                 modelShared, modelFrames, audioTrack);
    std::cout << modelShared;
    return 0;
}
