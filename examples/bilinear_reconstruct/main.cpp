#include "facedb/facedb.h"
#include "facedb/bilinear_model.h"
#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
#include "tools/projection.h"
#include "tools/contour.h"
#include "tools/distances.h"
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
    /* test distance */ {
        std::cout << "PointToLine:\n";
        std::cout << "sqr distance      " << PointToLine2D::sqrDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToLine2D::diffSqrDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "sqr distance      " << PointToLine2D::distance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToLine2D::diffDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "\n";

        std::cout << "PointToPoint:\n";
        std::cout << "sqr distance      " << Point2Point2D::sqrDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << Point2Point2D::diffSqrDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "sqr distance      " << Point2Point2D::distance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << Point2Point2D::diffDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "\n";
        
    }

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

    // projectToImageSpace(model.getMeshContourCands(0),
    //                     rsdevice.colorProjectionMat(), rsdevice.viewMat(), glm::mat4(1.0), landmarks);
    // auto contour_pair = getContourGrahamScan(landmarks);
    // auto contourIndex = model.getContourMeshIndex(contour_pair.second);
    // std::vector<snow::double3> contour3d;
    // for (size_t idx : contourIndex) {
    //     contour3d.push_back(model.meshVertex(0, idx));
    // }
    // projectToImageSpace(contour3d,
    //                     rsdevice.colorProjectionMat(), rsdevice.viewMat(), glm::mat4(1.0), landmarks);
    // auto contour = landmarks;
    // std::cout << contour.size() << std::endl;

    // set data
    win->setImage(rsdevice.colorImg());
    win->set2DLandmarks(landmarks);
    win->setPointCloud(rsdevice.pointCloud());
    win->setMorphModel(model.morphModel());
    // set mats
    win->setViewMat(rsdevice.viewMat());
    win->setProjMat(rsdevice.colorProjectionMat());

    app.addWindow(win);
    app.run();
    return 0;
}
