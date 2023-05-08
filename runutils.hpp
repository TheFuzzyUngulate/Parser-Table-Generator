#ifndef RUNUTILS_HPP
#define RUNUTILS_HPP
#pragma once

#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

struct flags {
    bool PARSER_TRACE = false;
    bool SCANNER_TRACE = false;
    bool PRINT_PARSE_TREE = false;
} flags;

void run_error(const char* ch) {
    std::cerr << ch << std::endl;
    exit(-1);
}

/**
 * @brief Handles flags. Currently supports parser tree syntax flag, scanner token flag, and debugger flag
 * 
 * @param args Argument list
 */
std::string handle_args(std::vector<std::string> args, int argcount) {
    std::string ret = "";

    if (args.size() == 0)
        run_error("insufficient arguments");
    if (args.size() > argcount + 1)
        run_error("too many arguments");

    std::string fname = args[0];
    args.erase(args.begin());
    
    for (auto str : args) {
        if (str == "-h") {
            std::cout << "usage: scan [-t] [-s] [-d] infile\n"
                      << "    infile:\tthe input file\n"
                      << "    -t:\t\tturn on parser trace\n"
                      << "    -s:\t\tturn on scanner trace\n"
                      << "    -d:\t\tdisplay syntax tree after parse\n";
            exit(-1);
        }
        else
        if (str == "-d") {
            flags.PRINT_PARSE_TREE = true;
        }
        else
        if (str == "-t") {
            flags.PARSER_TRACE = true;
        }
        else
        if (str == "-s") {
            flags.SCANNER_TRACE = true;
        }
        else
        if (str[0] == '-') 
            run_error("invalid flag provided.\nuse the \"-h\" (help) command to view options");
        else
        if (str != "" && ret != "")
            run_error("more than 1 file name provided");
        else
            ret = str;
    }

    if (ret == "")
        run_error("no file provided");
    
    return ret;
}

#endif