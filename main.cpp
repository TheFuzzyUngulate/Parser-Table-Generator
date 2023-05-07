#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "scanner.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "parseritems.hpp"

ast* tollist(vector<ast*> lst) {
    if (lst.size() <= 0) {
        cerr << "empty list given\n";
        exit(-1);
    }
    ast_el *head = new ast_el(lst[0]);
    for (int i = 1; i < lst.size(); ++i) {
        if (head->get_nxt() == nullptr) {
            head->addnext(lst[i]);
        }
    } return head;
}

bool ast_op1(vector<ast_rule*> a) {
    // terminate all instances of {a}, or something
    // this should actually change *ast
    // that is, A => {B} becomes A => B A1; A1 => B A1 | empty
    // return false if it is changed, so that 'simplify' can do an AND op on all 5 checks to comp
    bool ret = true;
    vector<ast_rule*> cpy = {};
    for (auto el : a) {
        // check if there exists a semi-exp of token type '{'
        // if it exists, remove it
        // have to use loops, oopsie
        vector<ast*> st;
        auto rhs = el->get_rhs();
        auto lhs = (lit*)el->get_lhs();
        bool found = false;

        for (int i = 0; i < rhs.size(); ++i) {
            if (rhs[i]->get_type() == "closed-expr") {
                auto new_tok = new lit(Tokens::RULE, lhs->get_lex() + "\'");

                vector<ast*> rhs1(i);
                std::copy_n(rhs.begin(), i, rhs1.begin());
                rhs1.push_back(new_tok);
                cpy.push_back(new ast_rule(lhs, rhs1));
                cpy.back()->print();


                ast_or *opt1;
                vector<ast*> rhs2(rhs.size()-i-1);
                if (rhs2.size() > 0) {
                    std::copy_n(rhs.begin()+i+1, rhs.size()-i-1, rhs2.begin());
                    opt1 = new ast_or(tollist(rhs2));
                    rhs2.clear();
                } else opt1 = new ast_or(new ast_el(new ast_empty()));
                opt1->setleft(new ast_el(new_tok));
                rhs[i]->getchlds().back()->print();
                opt1->setleft(rhs[i]->getchlds()[0]);
                cout << "we closed the pools?\n";
                rhs2.push_back(opt1);
                cpy.push_back(new ast_rule(new_tok, rhs2));
                found = true;
                cpy.back()->print();

                break;
            }
        }

        if (!found) cpy.push_back(el);
    }

    return false;
}

vector<ast_rule*> simplify(ast_rule *ast) {
    vector<ast_rule*> cpy{new ast_rule(*ast)};
    while (true) {
        // change the second copy
        // at the end, if it is still equal to the first, break

        if (ast_op1(cpy)) break;
    }
    return cpy;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Incorrect calling format\n";
        exit(-1);
    }

    std::ifstream myfile;
    myfile.open(argv[1]);
    if (!myfile.is_open()) {
        cerr << "File error\n";
        exit(-1);
    }

    Scanner *sc = new Scanner((std::fstream*)&myfile);
    Parser *par = new Parser(sc);
    par->parse();
    //par->print_root();

    vector<ast_rule*> cb1 = {};
    for (auto x : par->getroot()->getchlds())
        cb1.push_back((ast_rule*)x);
    ast_op1(cb1);

    return EXIT_SUCCESS;
}