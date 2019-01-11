#define TEST_ARGPARSE
#include <snow.h>
#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
    snow::ArgumentParser parser;

    parser.addArgument("data", 1, true);
    parser.addArgument("-a");
    parser.addArgument("--fuck");
    parser.addArgument("-v", "--visualize", snow::ArgumentParser::AtLeastOne, true, "visualize the app");
    parser.addArgument("test");

    parser.parse(argc, argv);

    std::cout << parser.get<bool>("a") << std::endl;
    std::cout << parser.get<bool>("fuck") << std::endl;
    std::cout << parser.getList<double>("visualize") << std::endl;

    return 0;
}
