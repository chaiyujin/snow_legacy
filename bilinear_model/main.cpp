
#include "facedb/facedb.h"
#include "visualize/show_model.h"

int main() {
    FaceDB::Initialize("../../assets/fw");

    snow::App app;
    app.addWindow(new ShowWindow());
    app.run();

    return 0;
}