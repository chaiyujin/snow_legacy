#include "visualizer/show.h"
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
    
    Image color(1920, 1080, 4);
    Image depth(640, 480, 2);
    readFrameBin("../../../assets/frame.bin", color.data(), depth.data());
    // win->setImage(&color);
    win->setDepth(&depth);

    app.addWindow(win);
    app.run();
    return 0;
}