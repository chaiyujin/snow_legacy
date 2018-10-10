#pragma once

#include <map>
#include <string>
#include <vector>
#include <sstream>

namespace snow {

struct Argument {
    bool                     mNeedValue;
    bool                     mRequired;
    std::vector<std::string> mValue;
};

class ArgumentParser {
    std::map<std::string, std::string> mShortNameMap;
    std::map<std::string, Argument>    mLongNameMap;
    int                                mPositionCount;
public:
    ArgumentParser() : mPositionCount(0) {}
    void addArgument(std::string name, char needValue=0, bool required=false) {
        std::string shortName = "";
        std::string longName = "";
        if (name.length() == 0) throw std::invalid_argument("empty argument name");

        if (name.length() < 2 || name[0] != '-') {
            // positional
            longName = std::string("--#") + std::to_string(mPositionCount);
            addArgument(shortName, longName, needValue, required);
        }
        else {
            if (name[0] == '-' && name[1] == '-')    longName = name;
            else                                     { shortName = name; longName = std::string("-") + shortName; }
            std::cout << "real add\n";
            addArgument(shortName, longName, needValue, required);
        }
    }
    void addArgument(std::string shortName, std::string longName, char needValue=0, bool required=false) {
        auto invalidChar = [](const std::string &name) -> bool {
            for (char c : name) {
                if (c == '-' || c == ' ' || c == '=' || c == '\\' || c == '#') return true;
            }
            return false;
        };
        Argument arg = {(needValue > 0), required, {}};
        if (shortName.length()) {
            if (shortName.length() < 2 || invalidChar(shortName.substr(1))) throw std::invalid_argument("invalid argument short name");
            mShortNameMap.insert({shortName.substr(1), longName.substr(2)});
            std::cout << "insert short name: " << shortName.substr(1) << " " << longName.substr(2) << std::endl;
        }
        if (longName.length() < 3  || invalidChar(longName.substr(2)))  throw std::invalid_argument("invalid argument long name");
        mLongNameMap.insert({longName.substr(2), arg});
        std::cout << "insert long name: " << longName.substr(2) << std::endl;
    }

    void parse(int argc, char **argv) {
        std::string argline = "";
        for (int i = 1; i < argc; ++i) { argline += std::string((i == 1)? "": " ") + argv[i]; }
        std::cout << argline << std::endl;

        std::map<std::string, std::string> valueMap;
        int len = (int)argline.length();
        auto validIndex = [&](int j) -> bool {
            return (j < len
                    && (argline[j] != ' ' || argline[j - 1] == '\\')
                    && (argline[j] != '=' || argline[j - 1] == '\\'));
        };
        auto subStr = [&](int l, int r) -> std::string {
            if (l < len && l < r) {
                std::string ret = argline.substr(l, r - l);
                ret = Replace(ret, "\\ ", " ");
                ret = Replace(ret, "\\=", "=");
                ret = Replace(ret, "\\-", "-");
                return ret;
            }
            return "";
        };
        auto readValue = [&](const std::string &key, int requireValue, int &i, int &j) -> std::string {
            std::string value = "";
            if (requireValue > 0) {
                if (j >= len || (argline[j] != ' ' && argline[j] != '=')) {
                    std::cout << "Failed to get value for `" << key << "`\n";
                    throw std::invalid_argument("Failed to get value.");
                }
                i = j + 1;
                j = i;
                if (j < len && argline[j] != '-') {
                    while (validIndex(j)) ++j;
                    value = subStr(i, j);
                    i = j;
                }
                else i = j-1;
                if (value.length() == 0) {
                    std::cout << "Failed to get value for `" << key << "`\n";
                    throw std::invalid_argument("Failed to get value.");
                }
            }
            else {
                if (j < len && argline[j] == '=') {
                    std::cout << "Assign value for `" << key << "` with `=`, however it doesn't need value.\n";
                    throw std::invalid_argument("Assign unnecessary value.");
                }
                i = j;
            }
            return value;
        };
        int countKeyValues = 0;
        int countPosValues = 0;
        auto addKeyValue = [&](const std::string &key, const std::string &value) {
            if (key.length() == 0) throw std::invalid_argument("empty key (only -- or only -) in args");
            std::cout << "'" << key << "' '" << value << "'" << std::endl;
            countKeyValues += 1;
        };
        auto addPosValue = [&](const std::vector<std::string> &value) {
            std::string key = std::string("#") + std::to_string(countPosValues);
            std::cout << "'" << key << "' '" << value << "'" << std::endl;
            auto it = mLongNameMap.find(key);
            if (it == mLongNameMap.end()) throw std::invalid_argument("too many positional args.");
            it->second.mValue = value;
            countPosValues += 1;
        };

        for (int i = 0; i < len; ++i) {
            int j;
            if (argline[i] == '-') {
                // key
                if (i + 1 < len && argline[i + 1] == '-') {
                    // long name
                    i += 2;
                    j = i;
                    while (validIndex(j)) ++j;
                    std::string key = subStr(i, j);
                    auto it = mLongNameMap.find(key);
                    if (it == mLongNameMap.end()) {
                        std::cout << "unknown " << key << std::endl;
                        throw std::invalid_argument("unknown key.");
                    }
                    int requireValue = it->second.mNeedValue;
                    // read value
                    std::string value = readValue(key, requireValue, i, j);
                    addKeyValue(key, value);
                }
                else {
                    // short name
                    i += 1;
                    j = i;
                    std::string key = "", value = "";
                    while (validIndex(j)) {
                        if (key.length() > 0) { addKeyValue(key, value); }
                        key = subStr(j, j+1);
                        ++j;

                        auto it = mShortNameMap.find(key);
                        if (it == mShortNameMap.end()) {
                            std::cout << "unknown " << key << std::endl;
                            throw std::invalid_argument("unknown key.");
                        }
                        int requireValue = mLongNameMap.find(it->second)->second.mNeedValue;
                        value = readValue(key, requireValue, i, j);
                        j = i;
                    }
                    // value = readValue(key, requireValue, i, j);
                    if (key.length() > 0) { addKeyValue(key, value); }
                }
            }
            else if (validIndex(i)) {
                // positional
                j = i;
                while (validIndex(j)) ++j;
                std::string value = subStr(i, j);
                i = j;
                addPosValue({value});
            }
        }

        // check for required
        for (auto it = mLongNameMap.begin(); it != mLongNameMap.end(); ++it) {
            if (it->second.mRequired && it->second.mValue.size() == 0) {
                std::cout << it->first << " is required, but not found.\n";
                throw std::invalid_argument("missing required argument.");
            }
        }
    }
};

void ParseArgs(int argc, char **argv) {
    std::string argline = "";
    for (int i = 1; i < argc; ++i) { argline += std::string((i == 1)? "": " ") + argv[i]; }
    std::cout << argline << std::endl;

    std::map<std::string, std::string> valueMap;
    int len = (int)argline.length();
    auto validIndex = [&](int j) -> bool {
        return (j < len
                && (argline[j] != ' ' || argline[j - 1] == '\\')
                && (argline[j] != '=' || argline[j - 1] == '\\'));
    };
    auto subStr = [&](int l, int r) -> std::string {
        if (l < len && l < r) {
            std::string ret = argline.substr(l, r - l);
            ret = Replace(ret, "\\ ", " ");
            ret = Replace(ret, "\\=", "=");
            ret = Replace(ret, "\\-", "-");
            return ret;
        }
        return "";
    };
    auto readValue = [&](const std::string &key, bool requireValue, int &i, int &j) -> std::string {
        std::string value = "";
        if (requireValue) {
            if (j >= len || (argline[j] != ' ' && argline[j] != '=')) {
                std::cout << "Failed to get value for `" << key << "`\n";
                throw std::invalid_argument("Failed to get value.");
            }
            i = j + 1;
            j = i;
            if (j < len && argline[j] != '-') {
                while (validIndex(j)) ++j;
                value = subStr(i, j);
                i = j;
            }
            else i = j-1;
            if (value.length() == 0) {
                std::cout << "Failed to get value for `" << key << "`\n";
                throw std::invalid_argument("Failed to get value.");
            }
        }
        else {
            if (j < len && argline[j] == '=') {
                std::cout << "Assign value for `" << key << "` with `=`, however it doesn't need value.\n";
                throw std::invalid_argument("Assign unnecessary value.");
            }
            i = j;
        }
        return value;
    };
    int countKeyValues = 0;
    int countPosValues = 0;
    auto addKeyValue = [&](const std::string &key, const std::string &value) {
        if (key.length() == 0) throw std::invalid_argument("empty key (only -- or only -) in args");
        std::cout << "'" << key << "' '" << value << "'" << std::endl;
        countKeyValues += 1;
    };
    auto addPosValue = [&](const std::string &value) {
        std::cout << "'pos " << countPosValues << "' '" << value << "'" << std::endl;
        countPosValues += 1;
    };

    bool requireValue = false;
    for (int i = 0; i < len; ++i) {
        int j;
        if (argline[i] == '-') {
            // key
            if (i + 1 < len && argline[i + 1] == '-') {
                // long name
                i += 2;
                j = i;
                while (validIndex(j)) ++j;
                std::string key = subStr(i, j);
                // read value
                std::string value = readValue(key, requireValue, i, j);
                addKeyValue(key, value);
            }
            else {
                // short name
                i += 1;
                j = i;
                std::string key = "", value = "";
                while (validIndex(j)) {
                    if (key.length() > 0) { addKeyValue(key, value); }
                    key = subStr(j, j+1);
                    ++j;
                    value = readValue(key, requireValue, i, j);
                    j = i;
                }
                // value = readValue(key, requireValue, i, j);
                if (key.length() > 0) { addKeyValue(key, value); }
            }
        }
        else if (validIndex(i)) {
            // positional
            j = i;
            while (validIndex(j)) ++j;
            std::string value = subStr(i, j);
            i = j;
            addPosValue(value);
        }
    }
}

}