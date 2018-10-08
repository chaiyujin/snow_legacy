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

static void readFrameBin(const char *filename, uint8_t *color, uint8_t *depth,
                     int colorSize=1920*1080*4, int depthSize=640*480*2) {
    FILE *fp = fopen(filename, "rb");
    size_t size;
    size = fread(color, 1, (size_t)colorSize, fp);// assert(size == colorSize);
    size = fread(depth, 1, (size_t)depthSize, fp);// assert(size == depthSize);
    fclose(fp);
}

int main() {
    FaceDB::Initialize("../../../assets/fw");
    librealsense::RealSenseSource rsdevice("../../../assets/test_depth/1-0-1.mkv_params_stream-1");
    glm::dmat4 PVM = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
    
    Image color(1920, 1080, 4);
    Image depth(640, 480, 2);
    readFrameBin("../../../assets/test_depth/frame.bin", color.data(), depth.data());
    // rsdevice.updateFramePair(color.data(), depth.data());
    // rsdevice.updatePointCloud();

    BilinearModel model;
    model.appendModel();
    model.prepareAllModel();

    FramesSolver solver;
    int Frames = 5;
    int Delta = 5;
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
    solver.solve(5);

    snow::App app;
    VisualizerWindow *win = new VisualizerWindow(75, rsdevice.depthImg().pixels(), FaceDB::NumVertices(), FaceDB::NumTriangles());

    // append data
    for (int iFrame = 0; iFrame < Frames; ++iFrame) {    
        solver.model().transformMesh(iFrame, glm::transpose(rsdevice.viewMat()));
        solver.model().updateMorphModel(iFrame);
        
        // win->appendImage(rsdevice.colorImg());
        win->append2DLandmarks(solver.landmarks(iFrame));
        win->appendMorphModel(solver.model().morphModel());
        // win->appendPointCloud(rsdevice.pointCloud());
    }
    // set mats
    win->setViewMat(rsdevice.viewMat());
    win->setProjMat(rsdevice.colorProjectionMat());

    app.addWindow(win);
    app.run();
    return 0;
}
