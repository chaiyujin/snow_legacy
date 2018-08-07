#include <snow.h>
#include <string>
#include "window.h"

int main() {
    snow::App app;
    app.addWindow(new ObjWindow("obj viewer"));
    app.run();
    return 0;
}
