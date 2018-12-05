#include "gui/window.h"

void usage() {
    std::cout << "Usage: AnimeViewer [obj | bilinear] [obj_path | fw_path]";
    exit(0);
}

void runObj(const char *objPath) {
    Application::newAPP(ModelType::Obj);
    Application::setObj("obj", objPath);
    Application::run(25.0);
    Application::terminate();
}

void runBilinear(const char *fwPath) {
    FaceDB::Initialize(fwPath);

    snow::WavPCM wavreader;
    wavreader.read("../../../assets/test.wav");
    std::vector<int16_t> wav(wavreader.channel(0).size());
    for (size_t i = 0; i < wavreader.channel(0).size(); ++i) {
        wav[i] = wavreader.channel(0)[i] * 32767.f;
    }

    std::vector<double> iden(75, 0);
    std::vector<double> expr(47, 0);
    iden[0] = 1.0; iden[1] = -1; iden[2] = 1;
    expr[0] = 1.0;
    std::vector<std::vector<double>> exprList(100, expr);
    for (int i = 1; i < 100; ++i) {
        exprList[i][i % 46 + 1] += 0.5;
    }

    Application::newAPP(ModelType::Bilinear);
    Application::addAudio("0", wav, wavreader.sampleRate());
    Application::setIden("bilinear", iden);
    Application::setExprList("bilinear", exprList);
    Application::setIden("bilinear2", iden);
    Application::setExprList("bilinear2", exprList);
    Application::setSubtitle("bilinear", "Fuck you you", 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
     5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6});
    // Application::offscreen("../../../assets/test_offscreen.mp4", 25.0, 1280, 480);
    Application::run(25.0);
    Application::terminate();
}

int main(int argc, char **argv) {
    if (argc < 3) usage();

    if (strcmp(argv[1], "obj") == 0) {
        runObj(argv[2]);
    }
    else if (strcmp(argv[1], "bilinear") == 0) {
        runBilinear(argv[2]);
    }
    else usage();

    return 0;
}
