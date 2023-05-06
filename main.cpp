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

vector<ast*> astlist_cpy(vector<ast*> orig) {
    vector<ast*> cpy = {};
    for (auto x : orig) {
        ast* y = new ast(*x);
        y->setchlds(vector<ast*>(x->getchlds()));
        if (y->get_type() == "ast-el") {
            ast_el* yptr = (ast_el*)y;
            yptr->set_ast(new ast(*yptr->get_ast()));
        }
        cpy.push_back(y);
    } return cpy;
}

bool ast_op1(vector<ast_rule*> a) {
    // terminate all instances of {a}, or something
    // this should actually change *ast
    // that is, A => {B} becomes A => B A1; A1 => B A1 | empty
    // return false if it is changed, so that 'simplify' can do an AND op on all 5 checks to comp
    bool ret = true;
    for (auto el : a) {
        // check if there exists a semi-exp of token type '{'
        // if it exists, remove it
        // have to use loops, oopsie
        vector<ast*> st;
        auto lhs = el->getchlds()[0];
        auto rhs = el->getchlds()[1];

        while (true) {
            
        }
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
    par->print_root();

    simplify((ast_rule*)par->apop()->getchlds()[0]);

    return EXIT_SUCCESS;
}