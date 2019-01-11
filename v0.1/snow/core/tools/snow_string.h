#pragma once
#include <string>
#include <algorithm>

namespace snow {
    // trim from start (in place)
    static inline void TrimBegin(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void TrimEnd(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !isspace(ch);
        }).base(), s.end());
    }

    static inline std::string Trim(std::string &s) {
        TrimBegin(s);
        TrimEnd(s);
        return s;
    }
    
    static inline std::string Replace(const std::string &s, const std::string &old, const std::string &replace) {
        std::string ret;
        for (size_t i = 0; i < s.length(); ) {
            if (i + old.length() <= s.length() && s.substr(i, old.length()) == old) {
                ret += replace;
                i += old.length();
            }
            else 
                ret += s[i++];
        }
        return ret;        
    }
}
