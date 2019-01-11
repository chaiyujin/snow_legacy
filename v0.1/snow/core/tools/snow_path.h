#pragma once
#include <vector>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#endif
#include <regex>

namespace snow {
namespace path {
inline bool Exists(const std::string &filename) {
#ifdef _WIN32
    return _access(filename.c_str(), 0) != -1;
#else
    return access(filename.c_str(), 0) != -1;
#endif
}

inline bool AllExists(const std::vector<std::string> &file_list) {
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
inline std::vector<std::string> FindFiles(const std::string &rootdir, const std::regex &pattern=std::regex("(.*)"), bool recursive=true, bool sort=false) {
#ifdef _WIN32
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
                if (std::regex_match(info.name, pattern)) {
                    std::string file_path = snow::path::Join(rootdir, info.name);
                    results.emplace_back(file_path);
                }
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
    if (sort) std::sort(results.begin(), results.end());
    return results;
#else
    std::vector<std::string> results;
    DIR *dir = nullptr;
    dirent *ent = nullptr;
    std::vector<std::string> subdirs;
    if ((dir = opendir(rootdir.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_type != DT_DIR) {
                if (std::regex_match(ent->d_name, pattern)) {
                    std::string file_path = snow::path::Join(rootdir, ent->d_name);
                    results.emplace_back(file_path);
                }
            }
            else if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                subdirs.push_back(ent->d_name);
            }
        }
        closedir(dir);
    }
    if (recursive) {
        for (size_t i = 0; i < subdirs.size(); ++i) {
            auto subResults = FindFiles(subdirs[i], pattern, recursive);
            for (size_t j = 0; j < subResults.size(); ++j) {
                results.emplace_back(snow::path::Join(rootdir, subResults[j]));
            }
        }
    }
    if (sort) std::sort(results.begin(), results.end());
    return results;
#endif
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
