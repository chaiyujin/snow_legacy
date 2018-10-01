#include "facedb/facedb.h"
#include "facedb/bilinear_model.h"
#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
#include "tools/projection.h"
#include "tools/contour.h"
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
    readFrameBin("../../../assets/test_depth/frame.bin", color.data(), depth.data());
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
        model.transformMesh(0, glm::transpose(rsdevice.viewMat()));
        model.updateMorphModel(0);
    }
    
    std::vector<snow::float2> landmarks;
    {
        int num;
        std::ifstream fin("../../../assets/test_depth/0-0-1.mkv_lmrecord");
        fin >> num >> num >> num;
        for (int i = 0; i < 73; ++i) {
            float x, y;
            fin >> x >> y;
            x = x * 2.f - 1.f;
            y = 1.f - y * 2.f;
            landmarks.push_back({ x, y });
        }
        fin.close();
    }
    
    // projectToImageSpace(model.mesh().data(0),
    //                     model.NumVertices(),
    //                     rsdevice.colorProjectionMat(), rsdevice.viewMat(), glm::mat4(1.0), landmarks);
    projectToImageSpace(model.getMeshContourCands(0),
                        rsdevice.colorProjectionMat(), rsdevice.viewMat(), glm::mat4(1.0), landmarks);
    auto contour = getContourGrahamScan(landmarks);
    std::cout << contour.size() << std::endl;

    // set data
    win->setImage(rsdevice.colorImg());
    win->set2DLandmarks(contour);
    win->setPointCloud(rsdevice.pointCloud());
    win->setMorphModel(model.morphModel());
    // set mats
    win->setViewMat(rsdevice.viewMat());
    win->setProjMat(rsdevice.colorProjectionMat());

    app.addWindow(win);
    app.run();
    return 0;
}
