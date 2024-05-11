#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <set>
#include <deque>
#include <string>

typedef struct
s_dirs
{
    int state;                            // state count
    std::string start;                    // name of starting state
    std::set<char> ignored;               // characters to be ignored.
    std::deque<std::pair<int, bool>> trs; // lines to be ignored by scanner
    int ateof;                            // token to be thrown before EOF
}
s_dirs;

#endif