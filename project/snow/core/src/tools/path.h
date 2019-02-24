#pragma once
#include <vector>
#include <string>
#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#endif

namespace snow { namespace path {

bool makedir(const std::string &dirname, bool exist_ok=true);

SNOW_INLINE bool makedirs(const std::string &dirname, bool exist_ok=true) {
    bool good = true;
    const size_t len = dirname.length();
    for (size_t i = 0; (i < len) && good; ++i) {
        if (dirname[i] == '/' || dirname[i] == '\\') {
            good &= path::makedir(dirname.substr(0, i), exist_ok);
        }
    }
    if (good) good &= path::makedir(dirname, exist_ok);
    return good;
}

SNOW_INLINE bool exists(const std::string &filepath) {
#ifdef WIN32
    return _access(filepath.c_str(), 0) != -1;
#else
    return access(filepath.c_str(), 0) != -1;
#endif
}

SNOW_INLINE bool exists(const std::vector<std::string> &files) {
    bool good = true;
    for (const std::string &filepath : files) {
        good &= exists(filepath);
        if (!good) break;
    }
    return good;
}

SNOW_INLINE std::pair<std::string, std::string> split_extension(const std::string &filepath) {
    size_t pos = filepath.find_last_of(".");
    if (pos != std::string::npos) {
        return {
            filepath.substr(0, pos),
            filepath.substr(pos)
        };
    } else {
        return {filepath, ""};
    }
}

SNOW_INLINE std::string extension(const std::string &filepath) {
    return split_extension(filepath).second;
}

SNOW_INLINE std::string change_extension(const std::string &filepath, const std::string &ext) {
    return split_extension(filepath).first + ext;
}

SNOW_INLINE std::string basename(const std::string &filepath, bool ext=true) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string basename = filepath.substr(pos+1);
        if (!ext) basename = split_extension(basename).first;
        return basename;
    } else {
        return filepath;
    }
}

SNOW_INLINE std::string dirname(const std::string &filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return filepath.substr(0, pos);
    } else {
        return ".";
    }
}

}

/* to not confuse with mkdir */
SNOW_INLINE bool path::makedir(const std::string &dirname, bool exist_ok) {
#ifdef WIN32
    int ret = _mkdir(dirname.c_str());
#else
    int ret = mkdir(dirname.c_str(), 0777); // notice that 777 is different than 0777
#endif
    return (ret == 0 || (ret == EEXIST && exist_ok));
}

}