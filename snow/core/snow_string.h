#pragma once
#include <string>
#include <algorithm>

namespace snow {
    // trim from start (in place)
    static inline void TrimBegin(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void TrimEnd(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static inline std::string Trim(std::string &s) {
        TrimBegin(s);
        TrimEnd(s);
        return s;
    }
}