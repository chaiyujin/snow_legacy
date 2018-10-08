#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <io.h>
#include <direct.h>

namespace snow {
namespace path {
inline bool Exists(const std::string &filename) {
    return _access(filename.c_str(), 0) != -1;
}

inline bool Exists(const std::vector<std::string> &file_list) {
    for (size_t i = 0; i < file_list.size(); ++i)
        if (!Exists(file_list[i])) return false;
    return true;
}

inline std::string Join(const std::string &dirname, const std::string &basename) {
    if (dirname.length() > 0) {
        if (dirname.back() != '/' && dirname.back() != '\\')
            return dirname + '/' + basename;
        else
            return dirname + basename;
    }
    else return basename;
}
inline std::vector<std::string> FindFiles(const std::string &rootdir, const std::string &pattern, bool recursive) {
    char currentPath[2048];
    _getcwd(currentPath, 2048);
    _chdir(rootdir.c_str());
    std::vector<std::string> results;
    // find files in this directory
    intptr_t handle;
    _finddata_t info;
    if ((handle = _findfirst("*", &info)) != -1) {
        // find the first
        do {
            // if not a directory, insert into result
            if (!(info.attrib & _A_SUBDIR)) {
                std::string file_path = snow::path::Join(rootdir, info.name);
                results.emplace_back(file_path);
                // printf("Find file: %s\n", file_path.c_str());
            }
        } while (_findnext(handle, &info) == 0);
        _findclose(handle);
    }
    // if recursive, find in sub directories.
    if (recursive) {
        std::vector<std::string> subDirs;
        if ((handle = _findfirst("*", &info)) != -1) {
            // find the first
            do {
                // sub dir
                if (info.attrib & _A_SUBDIR) {
                    if (strcmp(info.name, ".") &&
                        strcmp(info.name, ".."))
                        subDirs.emplace_back(info.name);
                }
            } while (_findnext(handle, &info) == 0);
            _findclose(handle);
        }
        for (size_t i = 0; i < subDirs.size(); ++i) {
            auto subResults = FindFiles(subDirs[i], pattern, recursive);
            for (size_t j = 0; j < subResults.size(); ++j) {
                results.emplace_back(snow::path::Join(rootdir, subResults[j]));
            }
        }
    }
    _chdir(currentPath);
    return results;
}

inline std::pair<std::string, std::string> SplitExtension(const std::string &filepath) {
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

inline std::pair<std::string, std::string> SplitBasename(const std::string &filepath) {
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

inline std::string Dirname(const std::string &filepath) {
    return SplitBasename(filepath).first;
}

inline std::string Basename(const std::string &filepath) {
    std::string ret = SplitBasename(filepath).second;
    if (ret.length() == 0) ret = filepath;
    return ret;
}

}}
