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
    const std::string RootVideo  = "../../../assets/test_depth/";

    auto fileList = snow::path::FindFiles(RootVideo, "*", true);
    for (auto &str: fileList) {
        std::cout << "Find file " << str << std::endl;
    }
    return 0;

    FaceDB::Initialize(RootFaceDB);
    librealsense::RealSenseSource rsdevice("../../../assets/test_depth/1-0-1.mkv_params_stream-1");
    glm::dmat4 PVM = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
    
    BilinearModel model;
    model.appendModel();
    model.prepareAllModel();

    FramesSolver solver;
    int Frames = 20;
    int Delta = 1;
    {
        std::ifstream fin("../../../assets/test_depth/1-0-1.mkv_lmrecord");
        for (int iFrame = 0; iFrame < Frames; ++iFrame) {
            Landmarks lms;
            for (int i = 0; i < Delta; ++i)
                fin >> lms;
            solver.addFrame(lms.landmarks(), PVM);
        }
        fin.close();
    }
    printf("begin to solve\n");
    solver.solve(5);

    snow::App app;
    VisualizerWindow *win = new VisualizerWindow(75, 0, FaceDB::NumVertices(), FaceDB::NumTriangles());

    // append data
    for (int iFrame = 0; iFrame < Frames; ++iFrame) {    
        solver.model().transformMesh(iFrame, glm::transpose(rsdevice.viewMat()));
        solver.model().updateMorphModel(iFrame);
        
        // win->appendImage(rsdevice.colorImg());
        win->append2DLandmarks(solver.landmarks(iFrame));
        win->appendMorphModel(solver.model().morphModel());
        // win->appendPointCloud(rsdevice.pointCloud());
        // set mats
        win->appendViewMat(rsdevice.viewMat());
        win->appendProjMat(rsdevice.colorProjectionMat());
    }

    app.addWindow(win);
    app.run();
    return 0;
}
