#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
#include "solver/frames_solver.h"
#include "solver/video_solver.h"
#include <snow.h>

std::string RootFaceDB = "";
std::string RootVideo  = "";

void solveIden(bool visualize, bool replace);
void solveVideo(std::string videoPath, bool visualize, bool replace);

int main(int argc, char **argv) {
    snow::ArgumentParser parser;
    parser.addArgument("type");
    parser.addArgument("--facedb",    1, true);
    parser.addArgument("--video_dir", 1, true);
    parser.addArgument("-v");
    parser.addArgument("-y");
    parser.parse(argc, argv);

    std::string type = parser.get<std::string>("type");
    RootFaceDB       = parser.get<std::string>("facedb");
    RootVideo        = parser.get<std::string>("video_dir");
    bool visualize   = parser.get<bool>("v");
    bool replace     = parser.get<bool>("y");

    FaceDB::Initialize(RootFaceDB);
    
    if (type == "iden")
        solveIden(visualize, replace);
    else {
        std::cout << "using regex: `" << type << "` to find videos." << std::endl;
        auto fileList = snow::path::FindFiles(RootVideo, std::regex(type), true);
        for (const auto &file : fileList)
            std::cout << "Find video " << file << std::endl;
        std::cout << "Totally " << fileList.size() << std::endl;
        for (size_t iFile = 0; iFile < fileList.size(); ++iFile) {
            std::cout << "[ " << iFile << " / " << fileList.size() << " ]: "
                      << snow::path::Basename(fileList[iFile]) << std::endl;
            solveVideo(fileList[iFile], visualize, replace);
        }
    }
    return 0;
}

void solveIden(bool visualize, bool replace) {
    std::string pathResult = snow::path::Join(RootVideo, "shared_params.txt");

    std::vector<glm::dmat4> viewMatList;
    std::vector<glm::dmat4> projMatList;
    
    FramesSolver solver;
    solver.setRegIden(1e-4);
    solver.setRegExpr(1e-4);
    bool ShowLandmarks = true;
    int Frames = 0;
    auto fileList = snow::path::FindFiles(RootVideo, std::regex("(\\d)-(\\d)-(\\d).mkv"), true);
    for (const auto &filePath: fileList) {
        auto pathParams    = filePath + "_params_stream-1";
        auto pathLandmarks = filePath + "_lmrecord";
        if (!snow::path::AllExists({pathParams, pathLandmarks})) continue;

        std::cout << "Find video: " << filePath << std::endl;
        glm::dmat4 PVM(1.0);
        /* read params */ {
            librealsense::RealSenseSource rsdevice(filePath + "_params_stream-1");
            PVM = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
            viewMatList.push_back(rsdevice.viewMat());
            projMatList.push_back(rsdevice.colorProjectionMat());
        }
        /* read first landmarks */ {
            std::ifstream fin(filePath + "_lmrecord");
            Landmarks lms; fin >> lms;
            solver.addFrame(lms, PVM);
            fin.close();
        }
        Frames ++;
    }
    if (Frames == 0) return;
    if (snow::path::Exists(pathResult) && !replace) {
        std::cout << "Result exists. use -y to overwrite iden." << std::endl;
        
        std::ifstream fin(pathResult);
        fin >> solver.model().scaleParameter()
            >> solver.model().idenParameter();
        fin.close();
        ShowLandmarks = false;
        Frames = 1;
    }
    else {
        printf("Begin to solve with %d frames\n", Frames);
        solver.solve(5, true);
        printf("Solve done.\n");
        {
            std::ofstream fout(pathResult);
            fout << solver.model().scaleParameter()
                 << solver.model().idenParameter();
            fout.close();
        }
    }
    if (visualize && Frames) {
        snow::App app;
        VisualizerWindow *win = new VisualizerWindow(75, 0, FaceDB::NumVertices(), FaceDB::NumTriangles());

        solver.model().prepareAllModel();
        solver.model().updateIdenOnCore(0);
        solver.model().updateExpr(0);
        solver.model().updateScale(0);
        solver.model().rotateYXZ(0);
        solver.model().translate(0);
        // visualization
        for (int iFrame = 0; iFrame < Frames; ++iFrame) {    
            solver.model().transformMesh(iFrame, glm::transpose(viewMatList[iFrame]));
            solver.model().updateMorphModel(iFrame);
            // append to visualizer        
            if (ShowLandmarks)
                win->append2DLandmarks(solver.landmarks(iFrame).data());
            win->appendMorphModel(solver.model().morphModel());
            win->appendViewMat(viewMatList[iFrame]);
            win->appendProjMat(projMatList[iFrame]);
        }
        
        app.addWindow(win);
        app.run();
    }
}

void solveVideo(std::string videoPath, bool visualize, bool replace) {
    auto pathParams    = videoPath + "_params_stream-1";
    auto pathLandmarks = videoPath + "_lmrecord";
    auto pathResult    = videoPath + "_pose_expr_params.txt";
    auto pathShared    = snow::path::Join(RootVideo, "shared_params.txt");
    if (!snow::path::AllExists({pathParams, pathLandmarks})) {
        std::cout << "Failed to find landmarks and camera params of video `" << videoPath << "`" << std::endl;
        return;
    }
    if (!snow::path::Exists(pathShared)) {
        std::cout << "Failed to find shared parameters" << std::endl;
        return;
    }
    glm::dmat4 pvmMat(1.0), viewMat(1.0), projMat(1.0);
    VideoSolver solver;
    /* read params */ {
        librealsense::RealSenseSource rsdevice(pathParams);
        pvmMat  = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
        viewMat = rsdevice.viewMat();
        projMat = rsdevice.colorProjectionMat();
        solver.setPVMMat(pvmMat);
    }
    /* read landmarks */ {
        std::ifstream fin(pathLandmarks);
        while (!fin.eof()) {
            Landmarks lms; fin >> lms;
            if (lms.timestamp() >= 0)
                solver.addFrame(lms);
        }
        fin.close();
    }
    /* read shared params */ {
        std::ifstream fin(pathShared);
        ScaleParameter scale;
        IdenParameter  iden;
        fin >> scale >> iden;
        solver.setSharedParameters(scale, iden);
        fin.close();
    }

    if (snow::path::Exists(pathResult) && !replace) {
        std::cout << "Result exists. use -y to overwrite" << std::endl;
        /* read existed results */ {
            std::ifstream fin(pathResult);
            int numResults = 0;
            std::string str;
            fin >> numResults; std::getline(fin, str);
            for (int iFrame = 0; iFrame < numResults; ++iFrame) {
                PoseParameter pose;
                ExprParameter expr;
                int64_t ts;
                fin >> ts; std::getline(fin, str);
                fin >> pose >> expr;
                solver.resultPoseList().emplace_back(pose);
                solver.resultExprList().emplace_back(expr);
            }
            fin.close();
        }
        std::cout << "read existed results " << solver.numResults() << std::endl;
    }
    else {
        std::cout << "Begin to solve " << solver.landmarksList().size() << " frames from video: " << videoPath << std::endl;
        solver.solve(5, false);
        std::cout << "Done." << std::endl;
        /* output results */ {
            std::ofstream fout(pathResult);
            fout << solver.numResults() << std::endl;
            for (int iFrame = 0; iFrame < solver.numResults(); ++iFrame) {
                fout << solver.landmarks(iFrame).timestamp() << std::endl
                     << solver.resultPoseList()[iFrame]
                     << solver.resultExprList()[iFrame];
            }
            fout.close();
        }
    }
    if (visualize) {
        snow::App app;
        VisualizerWindow *win = new VisualizerWindow(75, 0, FaceDB::NumVertices(), FaceDB::NumTriangles());

        // visualization
        int Frames = solver.numResults();
        solver.model().prepareAllModel();
        solver.model().updateIdenOnCore(0);
        for (int iFrame = 0; iFrame < Frames; ++iFrame) {
            printf("visualizer add %d frame\r", iFrame);
            solver.model().updateExpr(0, solver.resultExprList()[iFrame].param());
            solver.model().updateScale(0);
            solver.model().rotateYXZ(0, solver.resultPoseList()[iFrame].rotateYXZ());
            solver.model().translate(0, solver.resultPoseList()[iFrame].translate());
            solver.model().transformMesh(0, glm::transpose(viewMat));
            solver.model().updateMorphModel(0);
            // append to visualizer        
            win->append2DLandmarks(solver.landmarks(iFrame).data());
            win->appendMorphModel(solver.model().morphModel());
            win->appendViewMat(viewMat);
            win->appendProjMat(projMat);
        }
        
        app.addWindow(win);
        app.run();
    }
}
