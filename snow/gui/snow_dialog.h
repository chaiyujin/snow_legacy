/*
  Adapt from https://github.com/wjakob/nanogui/blob/master/src/darwin.mm
             https://github.com/wjakob/nanogui/blob/master/src/common.cpp
*/
#pragma once
#include <vector>
#include <string>

namespace snow {
    std::vector<std::string> FileDialog(const std::vector<std::pair<std::string, std::string>> &fileTypes, bool save, bool multiple);
    std::vector<std::string> FileDialog(const std::vector<std::string> &fileTypes, bool save, bool multiple);
}