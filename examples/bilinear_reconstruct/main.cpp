#include "visualizer/show.h"
#include "depth_source/realsense/rsutils.h"
#include <snow.h>

static void readFrameBin(const char *filename, uint8_t *color, uint8_t *depth,
                     int colorSize=1920*1080*4, int depthSize=640*480*2) {
    FILE *fp = fopen(filename, "rb");
    fread(color, 1, (size_t)colorSize, fp);
    fread(depth, 1, (size_t)depthSize, fp);
    fclose(fp);
}

int main() {
    snow::App app;
    ShowWindow *win = new ShowWindow();
    
    librealsense::RealSenseSoftwareDevice rsdevice("../../../assets/test_depth/0-0-1.mkv_params_stream-1");
    Image color(1920, 1080, 4);
    Image depth(640, 480, 2);
    readFrameBin("../../../assets/frame.bin", color.data(), depth.data());
    rsdevice.updateFramePair(color.data(), depth.data());
    rsdevice.updatePointCloud();
    win->setProjMat(rsdevice.colorProjection());
    win->setPointCloud(&rsdevice.pointCloud());
    win->setColor(&color);
    win->setDepth(&depth);

    app.addWindow(win);
    app.run();
    return 0;
}