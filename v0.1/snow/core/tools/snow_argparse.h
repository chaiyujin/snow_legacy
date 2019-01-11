#pragma once

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>
#include "snow_log.h"

namespace snow {

struct Argument {
    int                      mNeedValue;
    bool                     mRequired;
    bool                     mFind;
    std::string              mHelp;
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
    void addArgument(std::string name, char needValue=0, bool required=false, std::string help="") {
        std::string shortName = "";
        std::string longName = "";
        if (name.length() == 0) snow::fatal("empty argument name");
        if (name.length() < 2 || name[0] != '-') {
            // positional
            shortName = std::string("-#") + std::to_string(mPositionCount++);
            longName  = "--" + name;
            if (needValue == 0) needValue = 1;
            if (needValue == ArgumentParser::ZeroOrMore || needValue == ArgumentParser::AtLeastOne)
                snow::fatal("Positional argument {0} do not support `ZeroOrMore` and `AtLeastOne`.", name);
            addArgument(shortName, longName, needValue, required, help);
        }
        else {
            if (name[0] == '-' && name[1] == '-')    longName = name;
            else                                     { shortName = name; longName = std::string("-") + shortName; }
            addArgument(shortName, longName, needValue, required, help);
        }
    }
    void addArgument(std::string shortName, std::string longName, char needValue=0, bool required=false, std::string help="") {
        auto invalidName = [](const std::string &name) -> bool {
            if (name == "help") return true;
            for (char c : name) {
                if (c == '-' || c == ' ' || c == '=' || c == '\\'
                    || c == '(' || c == ')'
                    || c == '[' || c == ']'
                    || c == '{' || c == '}') return true;
            }
            return false;
        };
        Argument arg = {(int)needValue, required, false, help, {}};
        if (shortName.length()) {
            if (shortName.length() < 2 || invalidName(shortName.substr(1)))
                snow::fatal("invalid argument short name: {0}", shortName);
            if (mShortNameMap.find(shortName.substr(1)) != mShortNameMap.end())
                snow::fatal("short name {0} is duplicated.", shortName);
            mShortNameMap.insert({shortName.substr(1), longName.substr(2)});
#ifdef TEST_ARGPARSE
            std::cout << "insert short name: " << shortName.substr(1) << " " << longName.substr(2) << std::endl;
#endif
        }
        if (longName.length() < 3  || invalidName(longName.substr(2)))
            snow::fatal("invalid argument long name: {0}", longName);
        if (mLongNameMap.find(longName.substr(2)) != mLongNameMap.end())
            snow::fatal("long name {0} is duplicated.", longName);
        mLongNameMap.insert({longName.substr(2), arg});
#ifdef TEST_ARGPARSE
        std::cout << "insert long name: " << longName.substr(2) << std::endl;
#endif
    }

    void help(int argc, char **argv) {
#define POSPAIR std::pair<std::string, Argument>
#define VALTUPLE std::tuple<std::string, std::string, Argument>
        std::vector<POSPAIR>  posArgs;
        std::vector<VALTUPLE> keyArgs;
        std::map<std::string, int> flags;
        size_t maxNameLength = 0;
        for (auto it = mShortNameMap.begin(); it != mShortNameMap.end(); ++it) {
            auto arg = mLongNameMap.find(it->second)->second;
            if (it->first[0] == '#') {
                posArgs.push_back({it->first, arg});
            }
            else {
                VALTUPLE tuple;
                if (it->first == it->second) // only short
                    tuple = { it->first, "", arg };
                else // short long
                    tuple = { it->first, it->second, arg };
                keyArgs.push_back(tuple);
                if (it->second.length() > maxNameLength) maxNameLength = it->second.length();
            }
            flags.insert({it->second, 0});
        }
        for (auto it = mLongNameMap.begin(); it != mLongNameMap.end(); ++it) {
            if (flags.find(it->first) == flags.end()) {
                VALTUPLE tuple = { "", it->first, it->second };
                keyArgs.push_back(tuple);
                flags.insert({it->first, 0});
                if (it->first.length() > maxNameLength) maxNameLength = it->first.length();
            }
        }
        // sort pos args
        std::sort(posArgs.begin(), posArgs.end(), [](const POSPAIR &a, const POSPAIR &b)->bool{ return a.first < b.first; });
        std::sort(keyArgs.begin(), keyArgs.end(), [](const VALTUPLE &a, const VALTUPLE &b)->bool{
            return (std::get<2>(a).mRequired > std::get<2>(b).mRequired) ||
                   (std::get<2>(a).mRequired == std::get<2>(b).mRequired && std::get<0>(a) <  std::get<0>(b)) ||
                   (std::get<2>(a).mRequired == std::get<2>(b).mRequired && std::get<0>(a) == std::get<0>(b) && std::get<1>(a) < std::get<1>(b));
        });
        std::cout << "Usage: " << argv[0] << " ";
        for (const auto &pair : posArgs) {
            std::cout << mShortNameMap.find(pair.first)->second;
            switch (pair.second.mNeedValue) {
            case ZeroOrMore: std::cout << "(*) "; break;
            case AtLeastOne: std::cout << "(+) "; break;
            default:         std::cout << '(' << pair.second.mNeedValue << ") ";
            }
        }
        std::cout << std::endl;
        maxNameLength += 8;
        for (const auto &tup : keyArgs) {
            std::string shortName = std::get<0>(tup);
            std::string longName = std::get<1>(tup);
            Argument arg = std::get<2>(tup);
            std::string key;
            if (shortName.length() == 0) key = std::string("--") + longName;
            else if (longName.length() == 0) key = std::string("-") + shortName;
            else key = std::string("-") + shortName + ",--" + longName;
            {
                std::cout << "  " << std::setw(maxNameLength) << std::left << key;
                if (arg.mRequired) std::cout << "[required] "; else std::cout << "           ";
                switch (arg.mNeedValue) {
                case ZeroOrMore: std::cout << std::setw(8) << std::left << "(*)"; break;
                case AtLeastOne: std::cout << std::setw(8) << std::left << "(+)"; break;
                case 0         : std::cout << std::setw(8) << std::left << "(bool)"; break;
                default:         std::cout << std::setw(8) << std::left << '(' << arg.mNeedValue << ")";
                }
                std::cout << arg.mHelp;
                std::cout << std::endl;
            }
        }
#undef POSPAIR
#undef VALTUPLE
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
        if (iter == mLongNameMap.end())
            snow::fatal("unknown key `{}`", name);
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
                        if (requireValue > 0 || (requireValue == AtLeastOne))
                            snow::fatal("failed to get value for `{0}`", key);
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
                        if (requireValue > 0 || (requireValue == AtLeastOne))
                            snow::fatal("failed to get value for `{0}`", key);
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
                if (j < len && argline[j] == '=')
                    snow::fatal("assign value for `{0}` with `=`, however it doesn't need any more.", key);
            }
            return values;
        };
        int countKeyValues = 0;
        int countPosValues = 0;
        auto addKeyValue = [&](const std::string &key, const std::vector<std::string> &value) {
            if (key.length() == 0)
                snow::fatal("empty key (only -- or only -) in args");
            auto it = mLongNameMap.find(key);
            if (it->second.mFind)
                snow::fatal("`{0}` found twice", key);
            it->second.mFind = true;
            it->second.mValue = value;
            countKeyValues += 1;
        };
        auto addPosValue = [&](const std::string &key, const std::vector<std::string> &value) {
            auto it = mLongNameMap.find(key);
            if (it->second.mFind)
                snow::fatal("`{0}` found twice", key);
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
                    int requireValue = 0;
                    if (key == "help") {
                        help(argc, argv);
                        if (countPosValues == 0 && countKeyValues == 0) { exit(0); }
                    }
                    else {
                        auto it = mLongNameMap.find(key);
                        if (it == mLongNameMap.end())
                            snow::fatal("unknown key `{0}`", key);
                        requireValue = it->second.mNeedValue;
                    }
                    // read value
                    auto values = readValue(key, requireValue, i, j);
                    if (key != "help") addKeyValue(key, values);
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
                        if (it == mShortNameMap.end())
                            snow::fatal("unknown key `{0}`", key);
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
                if (countKeyValues > 0)
                    snow::fatal("positional arguments should be front of key-values.");
                std::string key = std::string("#") + std::to_string(countPosValues);
                auto it = mShortNameMap.find(key);
                if (it == mShortNameMap.end()) snow::fatal("too many positional args.");
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
            if (it->second.mRequired && !it->second.mFind)
                snow::fatal("`{0}` is required, but not found.", it->first);
#ifdef TEST_ARGPARSE
            std::cout << "key " << it->first << ": " << it->second.mValue << std::endl;
#endif
        }
    }
};

template <> inline bool ArgumentParser::get<bool>(const std::string &name) {
    auto iter = getIter(name);
    return iter->second.mFind;
}

template <typename T> inline T ArgumentParser::get(const std::string &name) {
    auto iter = getIter(name);
    if (iter->second.mValue.size() == 0)
        throw std::runtime_error("empty value");
    if (iter->second.mValue.size() > 1)
        snow::fatal("failed to get `{0}`, because it have values of size `{1:d}`", name, iter->second.mValue.size());
    std::istringstream ss(iter->second.mValue[0]);
    T ret;
    ss >> ret;
    return ret;
}

template <typename T> inline std::vector<T> ArgumentParser::getList(const std::string &name) {
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