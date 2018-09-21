#include "gui/window.h"
#include "obj/biwi_obj.h"

int main(int argc, char **argv) {
    FaceDB::Initialize("../../assets/fw/");
    if (argc < 2) {
        printf("Please give a .obj filepath.\n");
        return 0;
    }

    char path[256];
    std::vector<Vertices> anime;
    for (int i = 1; i <= 106; ++i) {
        sprintf(path, "../../assets/F1/01/frame_%03d.vl", i);
        anime.push_back(read_vl(path));
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
    Application::setIden("window0", iden);
    Application::setExprList("window0", exprList);
    Application::run(25.0);
    Application::terminate();

    return 0;
}
