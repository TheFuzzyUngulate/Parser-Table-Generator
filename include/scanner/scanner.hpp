#ifndef SCANNER_HPP
#define SCANNER_HPP

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "../utilities/runutils.hpp"
#include "../utilities/directives.hpp"

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

enum Tokens {
    LIT = 0,
    ARROW,
    BREAK,
    EMPTY,
    BAR,
    TOK,
    LOPT,
    ROPT,
    LREP,
    RREP,
    STRING,
    NEWLINE,
    ENDFILE,
    START,
    RULE,
    RULES,
    RULES1,
    RULESEL
};

inline const char* tokname(int tok) {
    switch(tok) {
        case Tokens::LIT: return "lit";
        case Tokens::TOK: return "tok";
        case Tokens::LOPT: return "[";
        case Tokens::ROPT: return "]";
        case Tokens::LREP: return "{";
        case Tokens::RREP: return "}";
        case Tokens::ARROW: return "=>";
        case Tokens::BREAK: return "linebreak";
        case Tokens::EMPTY: return "empty";
        case Tokens::ENDFILE: return "EOF";
        case Tokens::BAR: return "|";
        case Tokens::START: return "$start";
        case Tokens::RULE: return "$rule";
        case Tokens::RULES: return "$rules";
        case Tokens::RULES1: return "$rules\'";
        case Tokens::RULESEL: return "$rule_element";
        case Tokens::NEWLINE: return "newline";
        case Tokens::STRING: return "string";
        default:
            std::cerr << "Unknown token\n";
            exit(-1);
    }
}

class Scanner {
    public:
        Scanner(std::fstream *fptr) {
            file       = fptr;
            dirs.state = 0;
            prescan();
        }

        Tokens lex();
        void unlex(Tokens tok);
        int get();
        void prescan();
        void unget(int ch);
        void scan_err(const char* ch);
        void scan_warn(const char* ch);
        int getlineno() {return lineno;}
        string getlexeme() {return lexeme;}
        const s_dirs &sdir() {return dirs;}

    private:
        std::fstream *file;                         // A file containing rules
        string lexeme;                              // String content ("lexeme") of tokens like TOK and RULE
        int lineno = 1;                             // Current line number
        bool reached_end = false;                   // Boolean used to ensure EOF has a break inside of it
        vector<char> unget_list;                    // Vector storing unget characters for parser convenience
        s_dirs dirs;                                // Struct containing directives provided
        vector<Tokens> unlex_list;                  // SAVE ME AIEEEE
        
        void statetrans() 
        {
            if (!dirs.trs.empty()) 
            {
                auto top = dirs.trs.front();
                if (lineno == top.first)
                {
                    dirs.trs.pop_front();
                    if (top.second) {
                        dirs.state++;
                        unlex(ENDFILE);
                    }
                    char ch = get();
                    do {
                        ch = get(); 
                    } while (ch != '\n');
                    lineno++;
                }
            }
        }
};

#endif