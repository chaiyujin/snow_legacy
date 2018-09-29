#pragma once
#include <vector>
#include <fstream>
#include <string>

namespace snow {
namespace path {
inline bool exists(const std::string &filename) {
    return std::ifstream(filename).good();
}

inline bool exists(const std::vector<std::string> &file_list) {
    for (size_t i = 0; i < file_list.size(); ++i)
        if (!exists(file_list[i])) return false;
    return true;
}

inline std::pair<std::string, std::string> splitExtension(const std::string &filepath) {
    size_t pos = filepath.find_last_of('.');
    if (pos != std::string::npos) {
        std::string filename  = filepath.substr(0, pos);
        std::string extension = filepath.substr(pos+1);
        return {filename, extension};
    }
    else {
        return {filepath, ""};
    }
}

inline std::pair<std::string, std::string> splitBasename(const std::string &filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string dirpath  = filepath.substr(0, pos);
        std::string basename = filepath.substr(pos+1);
        return {dirpath, basename};
    }
    else {
        return {filepath, ""};
    }
}

inline std::string dirname(const std::string &filepath) {
    return splitBasename(filepath).first;
}

inline std::string basename(const std::string &filepath) {
    std::string ret = splitBasename(filepath).second;
    if (ret.length() == 0) ret = filepath;
    return ret;
}
}}
