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

    std::vector<double> iden(50, 0);
    std::vector<double> expr(47, 0);
    iden[0] = 1.0; iden[1] = -1; iden[2] = 1;
    expr[0] = 1.0;
    std::vector<std::vector<double>> exprList(100, expr);
    for (int i = 1; i < 100; ++i) {
        exprList[i][i % 46 + 1] += 0.5;
    }

    Application::newAPP(ModelType::Bilinear);
    Application::setIden("bilinear", iden);
    Application::setExprList("bilinear", exprList);
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
