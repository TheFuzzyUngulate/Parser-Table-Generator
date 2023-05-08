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
#include "runutils.hpp"

vector<ast*> tolist(ast_el* a) {
    vector<ast*> head = {};
    ast_el* clome = a;
    cout << "obv passed nere\n";
    while (clome != nullptr) {
        cout << "loop started\n";
        head.push_back(clome->get_ast());
        cout << "pushed back head\n";
        clome = (ast_el*)clome->get_nxt();
        cout << "loop complete\n";
    }
    return head;
}

ast* tolinkedlist(vector<ast*> lst) {
    if (lst.size() <= 0) {
        cerr << "empty list given\n";
        exit(-1);
    }
    ast_el *head = (lst[0]->get_tok() != Tokens::P_RULES_EL) 
                 ? new ast_el(lst[0]) 
                 : (ast_el*)lst[0];
    for (int i = 1; i < lst.size(); ++i) {
        if (head->get_nxt() == nullptr) {
            head->addnext(lst[i]);
        }
    } return head;
}

bool ast_op4(vector<ast_rule*> a) {
    // A => A B | C becomes A => C A1 and A1 => B A1 | empty
    return false;
}

bool ast_op3(vector<ast_rule*> &a) {
    // A => A B | A C becomes A => A A1 and A1 => B | C
    int prevsize = a.size();
    return false;
}

bool ast_op2(vector<ast_rule*> &a) {
    // A => A [B] C becomes A => A A' and A' => C | B C
    // A => [B] C becomes A => C | B C
    // A => [B] becomes A => B | empty
    // A => A B [C] becomes A => A B A' and A' => C | empty
    // so, if index is zero, don't create a new rule
    // if index is n-1, then the other alternative will be an "empty" rule
    int prevsize = a.size();
    vector<ast_rule*> cpy(a);
    a.clear();
    for (auto el : cpy) {
        auto rhs = el->get_rhs();
        auto lhs = (lit*)el->get_lhs();
        bool found = false;

        for (int i = 0; i < rhs.size(); ++i) {
            cout << tokname(rhs[i]->get_tok()) << std::endl;
            if (rhs[i]->get_type() == "closed-expr" && ((ast_el*)rhs[i])->get_ast()->get_tok() == Tokens::ROPT) {
                cout << "waht's opp\n";
                auto rhsi = (ast_in*)((ast_el*)rhs[i])->get_ast();
                auto new_tok = new lit(Tokens::RULE, lhs->get_lex() + "\'");

                vector<ast*> rhs1(i);
                std::copy_n(rhs.begin(), i, rhs1.begin());
                if (rhs1.size() > 0) {
                    rhs1.push_back(new ast_el(new_tok));
                    a.push_back(new ast_rule(lhs, rhs1));
                }
                
                ast_or *opt1;
                vector<ast*> rhs2(rhs.size()-i-1);
                if (rhs2.size() > 0) {
                    std::copy_n(rhs.begin()+i+1, rhs.size()-i-1, rhs2.begin());
                    opt1 = new ast_or(tolinkedlist(rhs2));
                    opt1->setleft(tolinkedlist(rhs2));
                    opt1->setleft(rhsi->getchlds()[0]);
                    rhs2.clear();
                } else {
                    opt1 = new ast_or(new ast_el(new ast_empty));
                    opt1->setleft(rhsi->getchlds()[0]);
                }

                rhs2.push_back(new ast_el(opt1));
                a.push_back(new ast_rule(new_tok, rhs2));
                found = true;
                break;
            }
        }

        if (!found)
            cpy.push_back(el);
    }

    return (a.size() == prevsize);
}

bool ast_op1(vector<ast_rule*> &a) {
    // A => {B} becomes A => B A1; A1 => B A1 | empty
    int prevsize = a.size();
    vector<ast_rule*> cpy(a);
    a.clear();
    for (auto el : cpy) {
        auto rhs = el->get_rhs();
        auto lhs = (lit*)el->get_lhs();
        bool found = false;

        for (int i = 0; i < rhs.size(); ++i) {
            if (rhs[i]->get_type() == "closed-expr" && ((ast_el*)rhs[i])->get_ast()->get_tok() == Tokens::RREP) {
                auto rhsi = (ast_in*)((ast_el*)rhs[i])->get_ast();
                auto new_tok = new lit(Tokens::RULE, lhs->get_lex() + "\'");

                vector<ast*> rhs1(i);
                std::copy_n(rhs.begin(), i, rhs1.begin());
                rhs1.push_back(new ast_el(new_tok));
                a.push_back(new ast_rule(lhs, rhs1));

                ast_or *opt1;
                vector<ast*> rhs2(rhs.size()-i-1);
                if (rhs2.size() > 0) {
                    std::copy_n(rhs.begin()+i+1, rhs.size()-i-1, rhs2.begin());
                    opt1 = new ast_or(tolinkedlist(rhs2));
                    rhs2.clear();
                } else opt1 = new ast_or(new ast_el(new ast_empty()));

                opt1->setleft(new ast_el(new_tok));
                opt1->setleft(rhsi->getchlds()[0]);

                rhs2.push_back(new ast_el(opt1));
                a.push_back(new ast_rule(new_tok, rhs2));
                found = true;

                break;
            }
        }

        if (!found) cpy.push_back(el);
    }

    a.clear();
    a = vector<ast_rule*>(cpy);
    return (a.size() == prevsize);
}

vector<ast_rule*> simplify(ast_rule *ast) {
    vector<ast_rule*> cpy{new ast_rule(*ast)};
    while (true) {
        if (ast_op1(cpy) && ast_op2(cpy)) break;
    }
    return cpy;
}

int main(int argc, char **argv) {

    vector<string> arguments(argv, argv+argc);
    string filename = handle_args(arguments, 3);

    std::ifstream myfile;
    myfile.open(filename);
    if (!myfile.is_open()) {
        run_error("unable to open file");
    }

    Scanner *sc = new Scanner((std::fstream*)&myfile);
    Parser *par = new Parser(sc);
    par->parse();
    if (flags.PRINT_PARSE_TREE)
        par->print_root();

    vector<ast_rule*> cb1 = {};
    for (auto x : par->getroot()->getchlds())
        cb1.push_back((ast_rule*)x);
    ast_op1(cb1);
    //ast_op2(cb1);

    for (auto x : cb1)
        x->print();

    return EXIT_SUCCESS;
}