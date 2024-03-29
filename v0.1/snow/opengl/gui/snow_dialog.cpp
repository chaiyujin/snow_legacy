#include <iostream>
#include <thread>
#include "snow_dialog.h"

namespace snow {
#if !defined(__APPLE__)
    std::vector<std::string> FileDialog(const std::vector<std::pair<std::string, std::string>> &fileTypes, bool save, bool multiple) {
        static const int FILE_DIALOG_MAX_BUFFER = 16384;
        std::vector<std::string> result;

        if (save && multiple) {
            snow::fatal("save and multiple must not both be true.");
            return result;
        }

#if defined(_WIN32)
        snow::fatal("not implemented.");
        return result;
    }
#else
        char buffer[FILE_DIALOG_MAX_BUFFER];
        buffer[0] = '\0';

        std::string cmd = "zenity --file-selection ";
        // The safest separator for multiple selected paths is /, since / can never occur
        // in file names. Only where two paths are concatenated will there be two / following
        // each other.
        if (multiple)
            cmd += "--multiple --separator=\"/\" ";
        if (save)
            cmd += "--save ";
        cmd += "--file-filter=\"";
        for (auto pair : fileTypes)
            cmd += "\"*." + pair.first + "\" ";
        cmd += "\"";
        FILE *output = popen(cmd.c_str(), "r");
        if (output == nullptr) snow::fatal("popen() failed -- could not launch zenity!");
        while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
            ;
        pclose(output);
        std::string paths(buffer);
        // paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());

        while (!paths.empty()) {
            size_t end = paths.find("//");
            if (end == std::string::npos) {
                std::string path = paths;
                Trim(path);
                if (path.length() > 0)
                    result.emplace_back(path);
                paths = "";
            } else {
                std::string path = paths.substr(0, end);
                Trim(path);
                if (path.length() > 0)
                    result.emplace_back(path);
                paths = paths.substr(end + 1);
            }
        }
        return result;
    } 
#endif  // _WIN32
#endif  // __APPLE__

    std::vector<std::string> FileDialog(const std::vector<std::string> &fileTypes, bool save, bool multiple) {
        std::vector<std::pair<std::string, std::string>> types;
        for (std::string str : fileTypes) {
            types.push_back(std::pair<std::string, std::string>(str, ""));
        }
        return FileDialog(types, save, multiple);
    }
}
