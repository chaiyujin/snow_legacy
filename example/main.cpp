#include <snow.h>
#include <string>
#include "window.h"

int main() {
    snow::App app;
    app.addWindow(new ObjWindow("obj viewer1"));
    app.addWindow(new ObjWindow("obj viewer2"));
    app.run();
    return 0;
}
