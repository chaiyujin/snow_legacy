#include <snow.h>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
    snow::ArgumentParser parser;

    parser.addArgument("-a");
    parser.addArgument("--fuck");
    parser.addArgument("-v", "--visualize", 1, true);

    parser.parse(argc, argv);
    // snow::ParseArgs(argc, argv);

    return 0;
}
