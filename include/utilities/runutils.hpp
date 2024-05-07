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
    bool PRINT_RULES = false;
    bool PRINT_RULE_GENERATION = false;
    bool PRINT_GRAMMAR = false;
    bool PRODUCE_GENERATOR = false;
    std::string input_file;
    std::string output_file;
};

void run_error(const char* ch);

/**
 * @brief Handles flags. Currently supports parser tree syntax flag, scanner token flag, and debugger flag
 * 
 * @param args Argument list
 */
inline struct flags handle_args(std::vector<std::string> args) {
    struct flags flags;

    if ((int)args.size() == 0)
        run_error("insufficient arguments");

    std::string fname = args[0];
    args.erase(args.begin());
    
    for (auto str : args) {
        if (str == "-h") {
            std::cout << "usage: scan [-t] [-s] [-d] infile\n"
                      << "    infile:\tthe input file\n"
                      << "    -t:\t\tturn on parser trace\n"
                      << "    -s:\t\tturn on scanner trace\n"
                      << "    -d:\t\tdisplay syntax tree after parse\n"
                      << "    -m:\t\tdisplay syntax tree simplification process\n"
                      << "    -r:\t\tdisplay rules extracted from syntax tree\n"
                      << "    -g:\t\tdisplay generated grammar with handles\n"
                      << "    -o:\t\tproduce parser tree generator\n"
                      << "    -a:\t\tactivate all flags\n";
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
        if (str == "-r") {
            flags.PRINT_RULES = true;
        }
        else
        if (str == "-g") {
            flags.PRINT_GRAMMAR = true;
        }
        else
        if (str == "-o") {
            flags.PRODUCE_GENERATOR = true;
        }
        else
        if (str == "-m") {
            flags.PRINT_RULE_GENERATION = true;
        }
        else
        if (str == "-a") {
            flags.PARSER_TRACE = true;
            flags.PRINT_GRAMMAR = true;
            flags.PRINT_PARSE_TREE = true;
            flags.PRINT_RULES = true;
            flags.PRODUCE_GENERATOR = true;
            flags.SCANNER_TRACE = true;
            flags.PRINT_RULE_GENERATION = true;

        }
        else
        if (str[0] == '-') 
            run_error("invalid flag provided.\nuse the \"-h\" (help) command to view options");
        else
        {
            if (flags.input_file == "")
                flags.input_file = str;
            else
            if (flags.output_file == "")
                flags.output_file = str;
            else run_error("too many file names provided");
        }
    }

    if (flags.input_file == "")
        run_error("no input file provided");
    if (flags.output_file == "")
        flags.output_file = "ptgparse";
    return flags;
}

#endif