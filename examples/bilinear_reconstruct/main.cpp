#include "facedb/facedb.h"
#include "facedb/bilinear_model.h"
#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
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
    librealsense::RealSenseSource rsdevice("../../../assets/test_depth/0-0-1.mkv_params_stream-1");

    snow::App app;
    VisualizerWindow *win = new VisualizerWindow(75, rsdevice.depthImg().pixels(), FaceDB::NumVertices(), FaceDB::NumTriangles());
    
    Image color(1920, 1080, 4);
    Image depth(640, 480, 2);
    readFrameBin("../../../assets/frame.bin", color.data(), depth.data());
    rsdevice.updateFramePair(color.data(), depth.data());
    rsdevice.updatePointCloud();

    BilinearModel model;
    {
        std::vector<double> iden(75, 0);
        std::vector<double> expr(47, 0);
        iden[0] = 1.0; iden[1] = -1; iden[2] = 1;
        expr[0] = 1.0; 
        model.updateIdenOnCore(0, iden.data());
        model.updateExpr(0, expr.data());
        model.updateScale(0);
        model.rotateYXZ(0);
        model.translate(0);
        model.transformMesh(0, rsdevice.viewMat());
        model.updateMorphModel(0);
    }
    
    std::vector<snow::float2> landmarks;
    {
        for (int i = 0; i < 73; ++i) {
            landmarks.push_back({ i / 73.f, i / 73.f });
        }
    }

    // set data
    win->setImage(rsdevice.colorImg());
    win->set2DLandmarks(landmarks);
    win->setPointCloud(rsdevice.pointCloud());
    win->setMorphModel(model.morphModel());
    // set mats
    win->setProjMat(rsdevice.colorProjectionMat());

    app.addWindow(win);
    app.run();
    return 0;
}
