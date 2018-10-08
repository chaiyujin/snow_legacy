#include "facedb/facedb.h"
#include "facedb/bilinear_model.h"
#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
#include "tools/contour.h"
#include "tools/landmarks.h"
#include "tools/math_tools.h"
#include "tools/projection.h"
#include "solver/cost_functions_2d.h"
#include "solver/frames_solver.h"
#include <snow.h>

int main() {
    const std::string RootFaceDB = "../../../assets/fw/";
    // const std::string RootVideo  = "D:/Projects/Recorder_qt5.6_sync/asset/000/";
    const std::string RootVideo  = "../../../assets/test_depth/";
    FaceDB::Initialize(RootFaceDB);
    
    std::vector<glm::dmat4> viewMatList;
    std::vector<glm::dmat4> projMatList;
    std::vector<glm::dmat4> pvmMatList;
    
    FramesSolver solver;
    int Frames = 0;
    // auto fileList = snow::path::FindFiles(RootVideo, std::regex("(\\d\\-\\d\\-\\d).mkv"), true);
    auto fileList = snow::path::FindFiles(RootVideo, std::regex("(\\d)-0-1.mkv"), true);
    for (const auto &filePath: fileList) {
        auto pathParams    = filePath + "_params_stream-1";
        auto pathLandmarks = filePath + "_lmrecord";
        if (!snow::path::AllExists({pathParams, pathLandmarks})) continue;

        std::cout << "Find video: " << filePath << std::endl;
        glm::dmat4 PVM(1.0);
        /* read params */ {
            librealsense::RealSenseSource rsdevice(filePath + "_params_stream-1");
            PVM = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
            pvmMatList.push_back(PVM);
            viewMatList.push_back(rsdevice.viewMat());
            projMatList.push_back(rsdevice.colorProjectionMat());
        }
        /* read first landmarks */ {
            std::ifstream fin(filePath + "_lmrecord");
            Landmarks lms;
            fin >> lms;
            solver.addFrame(lms.landmarks(), PVM);
            fin.close();
        }
        Frames ++;
    }
    if (Frames) {
        printf("Begin to solve with %d frames\n", Frames);
        solver.solve(5, true);
        printf("Solve done.\n");
    }
    if (Frames) {
        snow::App app;
        VisualizerWindow *win = new VisualizerWindow(75, 0, FaceDB::NumVertices(), FaceDB::NumTriangles());

        // visualization
        for (int iFrame = 0; iFrame < Frames; ++iFrame) {    
            solver.model().transformMesh(iFrame, glm::transpose(viewMatList[iFrame]));
            solver.model().updateMorphModel(iFrame);
            // append to visualizer        
            win->append2DLandmarks(solver.landmarks(iFrame));
            win->appendMorphModel(solver.model().morphModel());
            win->appendViewMat(viewMatList[iFrame]);
            win->appendProjMat(projMatList[iFrame]);
        }

        app.addWindow(win);
        app.run();
    }
    return 0;
}
