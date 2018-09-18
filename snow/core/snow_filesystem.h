#pragma once
#include <vector>
#include <fstream>

namespace snow {
    inline bool exists(const std::string &filename) {
        return std::ifstream(filename).good();
    }

    inline bool exists(const std::vector<std::string> &file_list) {
        for (size_t i = 0; i < file_list.size(); ++i)
            if (!exists(file_list[i]))
                return false;
        return true;
    }

}