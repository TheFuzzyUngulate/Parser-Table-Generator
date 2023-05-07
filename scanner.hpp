#ifndef SCANNER_HPP
#define SCANNER_HPP

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "runutils.hpp"

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

enum Tokens {ENDFILE = 0, TOK, LOPT, ROPT, LREP, RREP, ARROW, BREAK, EMPTY, RULE, BAR, P_START, P_RULE, P_RULES, P_RULES1, P_RULES_EL};

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

        int get() {
            if (!unget_list.empty()) {
                char ch = unget_list.back();
                unget_list.pop_back();
                return ch;
            }

            if (file->eof()) return 0;
            else {
                char res = 0;
                file->read(&res, 1);
                return res;       
            }
        }

        int getlineno() {return lineno;}
        string getlexeme() {return lexeme;}
        void unget(int ch) {unget_list.push_back(ch);}
        void scan_warn(const char* ch) {cerr << "scanner: " << ch << " at line " << lineno << std::endl;}
        void scan_err(const char* ch) {scan_warn(ch); exit(-1);}

        int lex() {
            while(1) {
                char ch;
                //lexeme.clear();

                do ch = get(); 
                while (ch != 0 && isspace(ch) && ch != '\n');

                switch(ch) {
                    case 0:
                        if (!reached_end) {
                            reached_end = true;
                            unget(ch);
                            return Tokens::BREAK;
                        } else return Tokens::ENDFILE;

                    case '[': return Tokens::LOPT;
                    case ']': return Tokens::ROPT;
                    case '{': return Tokens::LREP;
                    case '}': return Tokens::RREP;
                    case '|': return Tokens::BAR;
                    case '=':
                        ch = get();
                        if (ch == '>') return Tokens::ARROW;
                        else scan_err("invalid token");
                    
                    case '\n':
                        lineno++;
                        return Tokens::BREAK;
                    
                    case '\"':
                        lexeme.clear();
                        do {
                            ch = get();
                            lexeme += ch;
                        } while (isascii(ch) && ch != '\"');
                        lexeme.pop_back();
                        if (ch == '\"') return Tokens::TOK;
                        else scan_err("open quotation mark");
                        break;

                    default:
                        lexeme.clear();
                        if (isalpha(ch)) {
                            lexeme += ch;
                            
                            do {
                                ch = get();
                                lexeme += ch;
                            } while (isalnum(ch));
                            
                            lexeme.pop_back();
                            unget(ch);
                            
                            if (lexeme == "empty") return Tokens::EMPTY;
                            else return Tokens::RULE;
                        } else scan_err("invalid token");
                }
            }
        }

    private:
        std::fstream *file;
        string lexeme;
        int lineno = 1;
        bool reached_end = false;
        vector<char> unget_list;
};

#endif