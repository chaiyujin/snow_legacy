#pragma once

#include <map>
#include <string>
#include <vector>
#include <sstream>

namespace snow {

struct Argument {
    int                      mNeedValue;
    bool                     mRequired;
    bool                     mFind;
    std::vector<std::string> mValue;
};

/**
 * addArgument("-a")                            // bool
 * addArgument("--test")                        // bool
 * addArgument("-c", 1)                         // value
 * addArgument("-v", "--visualize", 2, true)    // 2 value, required
 * addArgument("data")                          // default 1 positional value
 * addArgument("data2", 2, true)                // required 2 positional value
 * */
class ArgumentParser {
    std::map<std::string, std::string> mShortNameMap;
    std::map<std::string, Argument>    mLongNameMap;
    int                                mPositionCount;
public:
    static const char ZeroOrMore = -2;
    static const char AtLeastOne = -1;

    ArgumentParser() : mPositionCount(0) {}
    void addArgument(std::string name, char needValue=0, bool required=false) {
        std::string shortName = "";
        std::string longName = "";
        if (name.length() == 0) throw std::invalid_argument("empty argument name");

        if (name.length() < 2 || name[0] != '-') {
            // positional
            shortName = std::string("-#") + std::to_string(mPositionCount);
            longName  = "--" + name;
            if (needValue == 0) needValue = 1;
            if (needValue == ArgumentParser::ZeroOrMore || needValue == ArgumentParser::AtLeastOne) {
                throw std::invalid_argument("Positional argument do not support `ZeroOrMore` and `AtLeastOne`.");
            }
            addArgument(shortName, longName, needValue, required);
        }
        else {
            if (name[0] == '-' && name[1] == '-')    longName = name;
            else                                     { shortName = name; longName = std::string("-") + shortName; }
            addArgument(shortName, longName, needValue, required);
        }
    }
    void addArgument(std::string shortName, std::string longName, char needValue=0, bool required=false) {
        auto invalidChar = [](const std::string &name) -> bool {
            for (char c : name) {
                if (c == '-' || c == ' ' || c == '=' || c == '\\') return true;
            }
            return false;
        };
        Argument arg = {(int)needValue, required, false, {}};
        if (shortName.length()) {
            if (shortName.length() < 2 || invalidChar(shortName.substr(1))) throw std::invalid_argument("invalid argument short name");
            if (mShortNameMap.find(shortName.substr(1)) != mShortNameMap.end()) {
                std::cout << shortName << " is duplicated." << std::endl;
                throw std::invalid_argument("duplicated.");
            }
            mShortNameMap.insert({shortName.substr(1), longName.substr(2)});
#ifdef TEST_ARGPARSE
            std::cout << "insert short name: " << shortName.substr(1) << " " << longName.substr(2) << std::endl;
#endif
        }
        if (longName.length() < 3  || invalidChar(longName.substr(2)))  throw std::invalid_argument("invalid argument long name");
        if (mLongNameMap.find(longName.substr(2)) != mLongNameMap.end()) {
            std::cout << longName << " is duplicated." << std::endl;
            throw std::invalid_argument("duplicated.");
        }
        mLongNameMap.insert({longName.substr(2), arg});
#ifdef TEST_ARGPARSE
        std::cout << "insert long name: " << longName.substr(2) << std::endl;
#endif
    }

    std::map<std::string, Argument>::iterator getIter(std::string name) {
        std::string oldName = name;
        auto iter = mLongNameMap.find(name);
        // failed to find in long
        if (iter == mLongNameMap.end()) {
            auto it = mShortNameMap.find(name);
            if (it != mShortNameMap.end()) name = it->second;
            iter = mLongNameMap.find(name);
        }
        if (iter == mLongNameMap.end()) {
            std::cout << "`" << oldName << "` not found in args." << std::endl;
            throw std::invalid_argument("argument not found.");
        }
        return iter;
    }

    template <typename T> T get(const std::string &name);
    template <typename T> std::vector<T> getList(const std::string &name);

    void parse(int argc, char **argv) {
        std::string argline = "";
        for (int i = 1; i < argc; ++i) { argline += std::string((i == 1)? "": " ") + argv[i]; }

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
        auto readValue = [&](const std::string &key, int requireValue, int &i, int &j) -> std::vector<std::string> {
            std::vector<std::string> values;
            i = j;
            if (requireValue != 0) {
                std::string value;
                while (true) {
                    if (j >= len || (argline[j] != ' ' && argline[j] != '=')) {
                        if (requireValue > 0 || (requireValue == AtLeastOne)) {
                            std::cout << "Failed to get value for `" << key << "`\n";
                            throw std::invalid_argument("Failed to get value.");
                        }
                        else break;
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
                        if (requireValue > 0 || (requireValue == AtLeastOne)) {
                            std::cout << "Failed to get value for `" << key << "`\n";
                            throw std::invalid_argument("Failed to get value.");
                        }
                        else break;
                    }
                    else {
                        values.push_back(value);
                        if (requireValue > 0) {
                            --requireValue;
                            if (requireValue == 0) break;
                        }
                        else {
                            if (requireValue == AtLeastOne) --requireValue;
                        }
                    }
                }
            }
            else {
                if (j < len && argline[j] == '=') {
                    std::cout << "Assign value for `" << key << "` with `=`, however it doesn't need value.\n";
                    throw std::invalid_argument("Assign unnecessary value.");
                }
            }
            return values;
        };
        int countKeyValues = 0;
        int countPosValues = 0;
        auto addKeyValue = [&](const std::string &key, const std::vector<std::string> &value) {
            if (key.length() == 0) throw std::invalid_argument("empty key (only -- or only -) in args");
            auto it = mLongNameMap.find(key);
            if (it->second.mFind) {
                std::cout << "Found twice " << key << std::endl;
                throw std::invalid_argument("twice");
            }
            it->second.mFind = true;
            it->second.mValue = value;
            countKeyValues += 1;
        };
        auto addPosValue = [&](const std::string &key, const std::vector<std::string> &value) {
            auto it = mLongNameMap.find(key);
            if (it->second.mFind) {
                std::cout << "Found twice " << key << std::endl;
                throw std::invalid_argument("twice");
            }
            it->second.mFind = true;
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
                    auto values = readValue(key, requireValue, i, j);
                    addKeyValue(key, values);
                }
                else {
                    // short name
                    i += 1;
                    j = i;
                    std::string key = "";
                    std::vector<std::string> values;
                    while (validIndex(j)) {
                        if (key.length() > 0) { addKeyValue(key, values); }
                        key = subStr(j, j+1);
                        ++j;

                        auto it = mShortNameMap.find(key);
                        if (it == mShortNameMap.end()) {
                            std::cout << "unknown " << key << std::endl;
                            throw std::invalid_argument("unknown key.");
                        }
                        int requireValue = mLongNameMap.find(it->second)->second.mNeedValue;
                        key = it->second;
                        values = readValue(key, requireValue, i, j);
                        j = i;
                    }
                    if (key.length() > 0) { addKeyValue(key, values); }
                }
            }
            else if (validIndex(i)) {
                // positional
                if (countKeyValues > 0) {
                    throw std::invalid_argument("positional arguments should be front of key-values.");
                }
                std::string key = std::string("#") + std::to_string(countPosValues);
                auto it = mShortNameMap.find(key);
                if (it == mShortNameMap.end()) throw std::invalid_argument("too many positional args.");
                auto itt = mLongNameMap.find(it->second);
                std::vector<std::string> values;
                key = itt->first;
                int numNeed = (int)itt->second.mNeedValue;
                while (numNeed--) {
                    j = i;
                    while (validIndex(j)) ++j;
                    std::string value = subStr(i, j);
                    i = j;
                    values.push_back(value);
                    if (numNeed) i++;
                }
                addPosValue(key, values);
            }
        }

        // check for required
        for (auto it = mLongNameMap.begin(); it != mLongNameMap.end(); ++it) {
            if (it->second.mRequired && !it->second.mFind) {
                std::cout << it->first << " is required, but not found.\n";
                throw std::invalid_argument("missing required argument.");
            }
#ifdef TEST_ARGPARSE
            std::cout << "key " << it->first << ": " << it->second.mValue << std::endl;
#endif
        }
    }
};

template <> bool ArgumentParser::get<bool>(const std::string &name) {
    auto iter = getIter(name);
    return iter->second.mFind;
}

template <typename T> T ArgumentParser::get(const std::string &name) {
    auto iter = getIter(name);
    if (iter->second.mValue.size() != 1) {
        std::cout << "failed to get `" << name << "`" << std::endl;
        throw std::invalid_argument("values size != 1");
    }
    std::istringstream ss(iter->second.mValue[0]);
    T ret;
    ss >> ret;
    return ret;
}

template <typename T> std::vector<T> ArgumentParser::getList(const std::string &name) {
    auto iter = getIter(name);
    std::vector<T> ret;
    for (const auto &str : iter->second.mValue) {
        std::istringstream ss(str);
        T val; ss >> val;
        ret.emplace_back(val);
    }
    return ret;
}

}