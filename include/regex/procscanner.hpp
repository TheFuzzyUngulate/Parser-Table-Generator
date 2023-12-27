#ifndef PROCSCANNER_HPP
#define PROCSCANNER_HPP

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../utilities/runutils.hpp"

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

enum RegExTokens {
    PS_ENDFILE = 0,
    PS_STRING,
    PS_CONTENT,
    PS_NEWLINE,
    PS_TRANSIT,
    PS_P_START,
    PS_P_RULE
};

inline const char* regtoknames(int tok) {
    switch(tok) {
        case RegExTokens::PS_ENDFILE: return "eof";
        case RegExTokens::PS_STRING: return "string";
        case RegExTokens::PS_CONTENT: return "regex";
        case RegExTokens::PS_NEWLINE: return "newline";
        case RegExTokens::PS_TRANSIT: return ":=";
        default:
            std::cerr << "Unknown token\n";
            exit(-1);
    }
}

class RegExScanner {
    public:
        RegExScanner(std::fstream *fptr) {
            file = fptr;
        }

        int lex();
        int get();
        void unget(int ch);
        void scan_err(const char* ch);
        void scan_warn(const char* ch);
        int getlineno() {return lineno;}
        string getlexeme() {return lexeme;}

    private:
        std::fstream *file;                         // A file containing rules
        string lexeme;                              // String content ("lexeme") of tokens like TOK and RULE
        int lineno = 1;                             // Current line number
        bool reached_end = false;                   // Boolean used to ensure EOF has a break inside of it
        vector<char> unget_list;                    // Vector storing unget characters for parser convenience
};

#endif