#include <snow/snow.h>
#include <string>
using namespace snow;

int main() {
    App app;
    app.addWindow("window0", new Window(1280, 720, "win0"));
    app.addWindow("window1", new Window(1280, 720, "win1"));
    app.run();
    return 0;
}
