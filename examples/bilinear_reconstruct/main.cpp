#include "facedb/facedb.h"
#include "facedb/bilinear_model.h"
#include "visualizer/window.h"
#include "depth_source/realsense/rsutils.h"
#include "tools/projection.h"
#include "tools/contour.h"
#include "tools/math_tools.h"
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

    /* test distance */ {
        std::cout << "PointToLine:\n";
        std::cout << "sqr distance      " << PointToLine2D::sqrDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToLine2D::diffSqrDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "sqr distance      " << PointToLine2D::distance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToLine2D::diffDistance(0.0, 0.0, 1.0, 0.0, 0.0, 1.0) << std::endl;
        std::cout << "\n";

        std::cout << "PointToPoint:\n";
        std::cout << "sqr distance      " << PointToPoint2D::sqrDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToPoint2D::diffSqrDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "sqr distance      " << PointToPoint2D::distance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "diff sqr distance " << PointToPoint2D::diffDistance(0.0, 0.0, 1.0, 1.0) << std::endl;
        std::cout << "\n";
        
    }

    FaceDB::Initialize("../../../assets/fw");
    librealsense::RealSenseSource rsdevice("../../../assets/test_depth/1-0-1.mkv_params_stream-1");

    snow::App app;
    VisualizerWindow *win = new VisualizerWindow(75, rsdevice.depthImg().pixels(), FaceDB::NumVertices(), FaceDB::NumTriangles());
    
    Image color(1920, 1080, 4);
    Image depth(640, 480, 2);
    readFrameBin("../../../assets/test_depth/frame.bin", color.data(), depth.data());
    rsdevice.updateFramePair(color.data(), depth.data());
    rsdevice.updatePointCloud();

    BilinearModel model;
    model.appendModel();
    model.prepareAllModel();

    std::vector<snow::float2> landmarks;
    {
        int num;
        std::ifstream fin("../../../assets/test_depth/1-0-1.mkv_lmrecord");
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
    // std::vector<size_t> contourIndex;
    // for (int epoch = 0; epoch < 5; ++epoch) {
    //     {
    //         model.updateIdenOnCore(0);
    //         model.updateExpr(0);
    //         // model.rotateYXZ(0);
    //         // model.translate(0);
    //         // model.transformMesh(0, glm::transpose(rsdevice.viewMat()));
    //         // model.updateMorphModel(0);
    //     }
    //     std::vector<glm::dvec2> landmark_contour;
    //     for (int i = 0; i < 15; ++i) { landmark_contour.push_back({ landmarks[i].x, landmarks[i].y }); }
    //     {
    //         // model.updateScale(0);
    //         glm::dmat4 pvm = rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat());
    //         ceres::Problem problem;
    //         for (size_t idx: contourIndex) {
    //             glm::dvec3 source3d = {
    //                 *model.tv11(0).data(idx * 3),
    //                 *model.tv11(0).data(idx * 3+1),
    //                 *model.tv11(0).data(idx * 3+2)
    //             };
    //             auto *cost = new PoseScaleCost2D(
    //                 nullptr, 0.0, &landmark_contour, 1.0, source3d, pvm);
    //             problem.AddResidualBlock(cost, nullptr,
    //                 model.poseParameter(0).trainRotateYXZ(),
    //                 model.poseParameter(0).trainTranslate(),
    //                 model.scaleParameter().train());
    //         }
    //         for (int iLM = 15; iLM < 73; ++iLM) {
    //             int idx = FaceDB::Landmarks73()[iLM];
    //             glm::dvec3 source3d = {
    //                 *model.tv11(0).data(idx * 3),
    //                 *model.tv11(0).data(idx * 3+1),
    //                 *model.tv11(0).data(idx * 3+2)
    //             };
    //             auto constraintP = glm::dvec2 {landmarks[iLM].x, landmarks[iLM].y};
    //             auto *cost = new PoseScaleCost2D(&constraintP, 1.0, nullptr, 0.0,
    //                     source3d, pvm);
    //             problem.AddResidualBlock(cost, nullptr,
    //                 model.poseParameter(0).trainRotateYXZ(),
    //                 model.poseParameter(0).trainTranslate(),
    //                 model.scaleParameter().train());
    //         }
    //         ceres::Solver::Options options;
    //         options.minimizer_progress_to_stdout = true;
    //         options.num_threads = 1;
    //         options.max_num_iterations = 10;
    //         ceres::Solver::Summary summary;
    //         ceres::Solve(options, &problem, &summary);
    //         // std::cout << model.poseParameter(0) << std::endl;
    //         // std::cout << model.scaleParameter() << std::endl;
    //         model.poseParameter(0).useTrained();
    //         model.scaleParameter().useTrained();
    //         // std::cout << model.poseParameter(0) << std::endl;
    //         // std::cout << model.scaleParameter() << std::endl;
    //     }
    //     if (epoch > 0) {
    //         {
    //             model.updateScale(0);
    //             model.rotateYXZ(0);
    //             model.translate(0);
    //         }
    //         {
    //             contourIndex = model.getContourIndex(0, rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat()));
    //         }
            
    //         ceres::Problem problem;
    //         glm::dmat4 pvm = (glm::dmat4)(rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat())) * model.poseParameter(0).matT() * model.poseParameter(0).matR();
    //         for (size_t idx: contourIndex) {
    //             auto *cost = new IdenExprScaleCostCost2D(
    //                 nullptr, 0.0, &landmark_contour, 1.0, FaceDB::CoreTensor(), idx, pvm);
    //             problem.AddResidualBlock(cost, nullptr,
    //                 model.idenParameter().train(),
    //                 model.exprParameter(0).train(),
    //                 model.scaleParameter().train());
    //         }
    //         for (int iLM = 15; iLM < 73; ++iLM) {
    //             int idx = FaceDB::Landmarks73()[iLM];
    //             auto constraint = glm::dvec2 {landmarks[iLM].x, landmarks[iLM].y};
    //             auto *cost = new IdenExprScaleCostCost2D(
    //                 &constraint, 1.0, nullptr, 0.0, FaceDB::CoreTensor(), idx, pvm);
    //             problem.AddResidualBlock(cost, nullptr,
    //                 model.idenParameter().train(),
    //                 model.exprParameter(0).train(),
    //                 model.scaleParameter().train());
    //         }
    //         {
    //             auto *regIden = new RegTerm(FaceDB::NumDimIden(), 0.0001);
    //             auto *regExpr = new RegTerm(FaceDB::NumDimExpr(), 0.0001);
    //             problem.AddResidualBlock(regIden, nullptr, model.idenParameter().train());
    //             problem.AddResidualBlock(regExpr, nullptr, model.exprParameter(0).train());
    //         }
    //         ceres::Solver::Options options;
    //         options.minimizer_progress_to_stdout = true;
    //         options.num_threads = 1;
    //         options.max_num_iterations = 10;
    //         ceres::Solver::Summary summary;
    //         ceres::Solve(options, &problem, &summary);

    //         model.idenParameter().useTrained();
    //         model.exprParameter(0).useTrained();
    //         model.scaleParameter().useTrained();
    //     }
    //     {
    //         model.updateIdenOnCore(0);
    //         model.updateExpr(0);
    //         model.updateScale(0);
    //         model.rotateYXZ(0);
    //         model.translate(0);
    //     }
    //     {
    //         contourIndex = model.getContourIndex(0, rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat()));
    //     }
    // }
    // {
    //     model.updateIdenOnCore(0);
    //     model.updateExpr(0);
    //     model.updateScale(0);
    //     model.rotateYXZ(0);
    //     model.translate(0);
    //     model.transformMesh(0, glm::transpose(rsdevice.viewMat()));
    //     model.updateMorphModel(0);
    // }
    
    // std::vector<snow::double3> contour3d;
    // for (size_t idx : contourIndex) {
    //     contour3d.push_back(model.meshVertex(0, idx));
    // }
    // projectToImageSpace(contour3d,
    //                     rsdevice.colorProjectionMat(), rsdevice.viewMat(), glm::mat4(1.0), landmarks);
    // auto contour = landmarks;
    // std::cout << contour.size() << std::endl;


    FramesSolver solver;
    solver.addFrame(landmarks);
    solver.solve(5, rsdevice.colorProjectionMat() * rsdevice.viewMat() * glm::transpose(rsdevice.viewMat()));

    solver.model().transformMesh(0, glm::transpose(rsdevice.viewMat()));
    solver.model().updateMorphModel(0);

    // set data
    win->setImage(rsdevice.colorImg());
    win->set2DLandmarks(landmarks);
    // win->setPointCloud(rsdevice.pointCloud());
    win->setMorphModel(solver.model().morphModel());
    // set mats
    win->setViewMat(rsdevice.viewMat());
    win->setProjMat(rsdevice.colorProjectionMat());

    app.addWindow(win);
    app.run();
    return 0;
}
