#pragma once
#include <vector>
#include <string>

namespace snow {
    std::vector<std::string> FileDialog(const std::vector<std::pair<std::string, std::string>> &fileTypes, bool save, bool multiple);
}