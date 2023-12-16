#ifndef PARSER_HPP
#define PARSER_HPP

#pragma once

#include <deque>

#include "../ast/ast.hpp"
#include "parseritems.hpp"
#include "../scanner/scanner.hpp"
#include "../utilities/runutils.hpp"
#include "../utilities/general_utils.hpp"

using   std::cout, 
        std::cerr, 
        std::string, 
        std::vector, 
        std::make_pair,
        std::deque;

class Parser {
    public:
        Parser(Scanner* scptr, struct flags flags) {
            sc = scptr;
            _flags = flags;
            prime_table();
        }

        void ppush(ParserItem* p);
        ParserItem* ppop();
        void pprint();

        void ast_push(AST* a);
        AST* ast_pop();
        void ast_print();
        
        bool is_terminal(Tokens t) {return t < P_START;}
        void parse_warn(const char* ch);
        void parse_err(const char* ch);
        void parse_unexpected_terminal_err(Tokens t_exp, Tokens t_rec);
        void parse_illegal_transition_err(Tokens t_tos, Tokens t_cur);
        
        void print_root();
        StartAST* getroot() {return root;}

        int prime_table();
        void print_dict();
        int parse();

    private:
        Scanner *sc;
        ParseDict dict;
        vector<AST*> res_stack;
        vector<AST> true_res_stack;
        vector<ParserItem*> pred_stack;
        StartAST* root;
        struct flags _flags;
};

#endif
