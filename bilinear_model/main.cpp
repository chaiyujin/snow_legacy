
#include "faceware/faceware.h"
#include "faceware/show_model.h"

int main() {
    FaceDB::read_raw_information("../../assets/fw");

    snow::App app;
    app.addWindow(new ShowWindow());
    app.run();
    
    return 0;
}