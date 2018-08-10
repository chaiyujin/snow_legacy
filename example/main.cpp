#include <snow.h>
#include <string>
#include "window.h"

int main() {
    snow::App app;
    app.addWindow(new ObjWindow("obj viewer"));
    app.addWindow(new CameraWindow("camera window"));
    app.run();
    return 0;
}
