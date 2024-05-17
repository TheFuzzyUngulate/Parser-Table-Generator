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
    ID,
    COLON,
    COMMA,
    LIT,
    ARROW,
    BREAK,
    SKIP,
    GOTO,
    IN,
    AFTER,
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
    START1,
    RULE,
    OPTS,
    RULES,
    RULES1,
    RULESEL,
    COMS,
    COMS1,
    COMM
};

inline const char* tokname(int tok) {
    switch (tok)
    {
        case Tokens::ID: return "id";
        case Tokens::COLON: return ":";
        case Tokens::COMMA: return ",";
        case Tokens::LIT: return "lit";
        case Tokens::ARROW: return "=>";
        case Tokens::BREAK: return "break";
        case Tokens::SKIP: return "skip";
        case Tokens::GOTO: return "goto";
        case Tokens::IN: return "in";
        case Tokens::AFTER: return "after";
        case Tokens::EMPTY: return "empty";
        case Tokens::BAR: return "|";
        case Tokens::TOK: return "tok";
        case Tokens::LOPT: return "[";
        case Tokens::ROPT: return "]";
        case Tokens::LREP: return "{";
        case Tokens::RREP: return "}";
        case Tokens::STRING: return "string";
        case Tokens::NEWLINE: return "newline";
        case Tokens::ENDFILE: return "EOF";
        case Tokens::START: return "$START";
        case Tokens::START1: return "$START\'";
        case Tokens::OPTS: return "$OPTS";
        case Tokens::RULE: return "$RULE";
        case Tokens::RULES: return "$RULES";
        case Tokens::RULES1: return "$RULES\'";
        case Tokens::RULESEL: return "$RULSEL";
        case Tokens::COMS: return "$COMS";
        case Tokens::COMS1: return "$COMS\'";
        case Tokens::COMM: return "$COMM";
        default:
            std::cerr << "Unknown token\n";
            exit(-1);
    }
}

class Scanner {
    public:
        Scanner(std::fstream *fptr) {
            file = fptr;
            dirs.state = 0;
            dirs.ateof = "";
            dirs.nodecollapse = false;
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
                while (true)
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
                    } else break;
                }
            }
        }
};

#endif