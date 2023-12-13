#ifndef GENERAL_UTILS_HPP
#define GENERAL_UTILS_HPP
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

bool isalpha(std::string str) {
    for (auto x : str)
        if (!isalpha(x)) return false;
    return true;
}

bool isdigit(std::string str) {
    for (auto x : str)
        if (!isdigit(x)) return false;
    return true;
}

bool isalnum(std::string str) {
    for (auto x : str)
        if (!isalnum(x)) return false;
    return true;
}

std::string upper(std::string str) {
    std::string res;
    for (int i = 0; i < str.size(); ++i)
        res += std::toupper(str[i]);
    return res;
}

#endif