#ifndef SCANNER_HPP
#define SCANNER_HPP

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../utilities/runutils.hpp"

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

enum Tokens {
    ENDFILE = 0,
    TOK,
    LOPT,
    ROPT,
    LREP,
    RREP,
    ARROW,
    BREAK,
    EMPTY,
    RULE,
    BAR,
    P_START,
    P_RULE,
    P_RULES,
    P_RULES1,
    P_RULES_EL
};

inline const char* tokname(int tok) {
    switch(tok) {
        case Tokens::RULE: return "rule";
        case Tokens::TOK: return "token";
        case Tokens::LOPT: return "[";
        case Tokens::ROPT: return "]";
        case Tokens::LREP: return "{";
        case Tokens::RREP: return "}";
        case Tokens::ARROW: return "=>";
        case Tokens::BREAK: return "linebreak";
        case Tokens::EMPTY: return "empty";
        case Tokens::ENDFILE: return "EOF";
        case Tokens::BAR: return "|";
        case Tokens::P_START: return "$start";
        case Tokens::P_RULE: return "$rule";
        case Tokens::P_RULES: return "$rules";
        case Tokens::P_RULES1: return "$rules\'";
        case Tokens::P_RULES_EL: return "$rule_element";
        default:
            std::cerr << "Unknown token\n";
            exit(-1);
    }
}

class Scanner {
    public:
        Scanner(std::fstream *fptr) {
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