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
    S_TRANSIT,
    S_NEWLINE,
    S_DELIM,
    S_CONTENT,
    S_STRING,
    M_START,
    M_START1,
    P_START,
    P_RULE,
    P_RULES,
    P_RULES1,
    P_RULES_EL,
    S_RULE,
    S_START
};

inline const char* tokname(int tok) {
    switch(tok) {
        case Tokens::M_START: return "main_start";
        case Tokens::M_START1: return "main_start1";
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
        case Tokens::S_TRANSIT: return ":=";
        case Tokens::S_NEWLINE: return "linebreak";
        case Tokens::S_DELIM: return "%%";
        case Tokens::S_CONTENT: return "content";
        case Tokens::S_STRING: return "string";
        case Tokens::S_RULE: return "$scan_rule";
        case Tokens::S_START: return "$scan_start";
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

        Tokens lex();
        int get();
        void unget(int ch);
        void scan_err(const char* ch);
        void scan_warn(const char* ch);
        int getlineno() {return lineno;}
        string getlexeme() {return lexeme;}
        string getstartstate() {return start_state;}

    private:
        int state = 0;                              // Scanner's current state
        std::fstream *file;                         // A file containing rules
        string lexeme;                              // String content ("lexeme") of tokens like TOK and RULE
        int lineno = 1;                             // Current line number
        bool reached_end = false;                   // Boolean used to ensure EOF has a break inside of it
        vector<char> unget_list;                    // Vector storing unget characters for parser convenience
        string start_state;                         // Start state value found by scanner
};

#endif