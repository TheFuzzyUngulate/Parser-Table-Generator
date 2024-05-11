#ifndef PARSER_HPP
#define PARSER_HPP

#pragma once

#include <deque>
#include <exception>
#include <optional>

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
        std::tuple,
        std::deque;

typedef deque<std::pair<string, string>> regopts;
typedef tuple<string, string, regopts> reglit;

class Parser {
    public:
        Parser(Scanner* scptr, struct flags flags) {
            sc = scptr;
            _flags = flags;
            //prime_table();
        }
        
        bool isterminal(Tokens t) {return t < START;}
        
        void parse_warn(const char* ch);
        void parse_err(const char* ch);
        void parse_unexpected_terminal_err(Tokens t_exp, Tokens t_rec);
        void parse_illegal_transition_err(Tokens t_tos, Tokens t_cur);

        bool sc_exists(string name);
        void parse();
        void pr_parse();
        void sc_parse();

    private:
        Scanner *sc;
        ParseDict dict;
        vector<AST*> res_stack;
        vector<string> reg_stack;
        vector<AST> true_res_stack;
        vector<ParserItem*> pred_stack;
        StartAST* root;
        StartAST* regexes;
        struct flags _flags;
        deque<Rule*> _psitems;
        deque<reglit> _scitems;

    public:
        const deque<Rule*>& psitems(){return _psitems;}
        const deque<reglit>& scitems(){return _scitems;}
};

/* NEWER CONTENT */

typedef std::map<std::pair<Tokens, Tokens>, std::vector<Tokens>> ParserTable;

typedef
struct pitem {
    enum {
        LIT,
        NONLIT,
        REDUCTION
    } id;
    union {
        Tokens lit;
        Tokens nonlit;
        struct {
            Tokens tag;
            std::size_t count;
        } reduce;
    } op;
} pitem;

static inline string
pitem_string(pitem p)
{
    std::string str;

    switch (p.id)
    {
        case pitem::LIT:
            str += "lit(";
            str += tokname(p.op.lit);
            str += ")";
            break;

        case pitem::NONLIT:
            str += "nonlit(";
            str += tokname(p.op.nonlit);
            str += ")";
            break;

        case pitem::REDUCTION:
            str += "reduce(";
            str += tokname(p.op.reduce.tag);
            str += ", ";
            str += std::to_string(p.op.reduce.count);
            str += ")";
            break;
    }

    return str;
}

#endif