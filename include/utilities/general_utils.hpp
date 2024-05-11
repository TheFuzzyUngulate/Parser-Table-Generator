#ifndef GENERAL_UTILS_HPP
#define GENERAL_UTILS_HPP
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#define ch_to_str(ch) ((ch) == '\n' ? "\\n" : ((ch) == '\t' ? "\\t" : ((ch) == '\r' ? "\\r" : ((ch) == '\\' ? "\\\\" : ((ch) == '\"' ? "\\\"" : ((ch) == '\'' ? "\\\'" : std::string(1, (ch))))))))

inline bool islit(std::string str) {
    if (!isalpha(str[1])) return false;
    for (int i = 1; i < str.size(); ++i)
        if (!isalnum(str[i]) && str[i] != '_') return false;
    return true;
}

inline bool isalpha(std::string str) {
    for (auto x : str)
        if (!isalpha(x)) return false;
    return true;
}

inline bool isdigit(std::string str) {
    for (auto x : str)
        if (!isdigit(x)) return false;
    return true;
}

inline bool isalnum(std::string str) {
    for (auto x : str)
        if (!isalnum(x)) return false;
    return true;
}

inline std::string upper(std::string str) {
    std::string res;
    for (int i = 0; i < str.size(); ++i)
        res += std::toupper(str[i]);
    return res;
}

inline std::string ltrim(std::string src, std::string delim) {
    std::string res = src;
    for (int i = 0; i < src.size(); i += delim.size()) {
        if (src.substr(0, delim.size()) == delim)
            res = res.substr(delim.size());
        else break;
    } return res;
}

inline std::string rtrim(std::string src, std::string delim) {
    std::string res = src;
    for (int i = src.size()-1; i > -1; i -= delim.size()) {
        if (src.substr(src.size() - 1 - delim.size(), delim.size()) == delim)
            res = res.substr(0, src.size() - delim.size());
        else break;
    } return res;
}

inline std::string trim(std::string src, std::string delim) {
    return ltrim(rtrim(src, delim), delim);
}

#endif