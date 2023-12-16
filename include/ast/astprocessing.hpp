#ifndef ASTPROCESSING_HPP
#define ASTPROCESSING_HPP
#pragma once

#include "ast.hpp"
#include <set>

class ASTProcessor {
    public:
        ASTProcessor(StartAST* start) {
            _start = start;
        }

        /*bool rule_eq(Rule* r1, Rule* r2) {
            if (r1->getLeft()->getName() != r2->getLeft()->getName())
                return false;
            auto r1_ch = r1->getRight()->getChildren();
            auto r2_ch = r2->getRight()->getChildren();
            if (r1_ch.size() != r2_ch.size())
                return false;
            for (int i = 0; i < (int)r1_ch.size(); i++) {
                if (r1_ch[i]->getId() != r2_ch[i]->getId())
                    return false;
                if (r1_ch[i]->getId() == "empty")
                    break;
                Literal* lit1 = (Literal*)r1_ch[i];
                Literal* lit2 = (Literal*)r2_ch[i];
                if (lit1->getName() != lit2->getName())
                    return false;
            }
            return true;
        }*/

        /**
         * Search for and remove duplicate rules
        */
        deque<AST*> trans6(deque<AST*> start);
        std::pair<bool, deque<AST*>> trans5(deque<AST*> start);
        std::pair<bool, deque<AST*>> trans4(deque<AST*> start);
        std::pair<bool, deque<AST*>> trans3(deque<AST*> start);
        std::pair<bool, deque<AST*>> trans2(deque<AST*> start);
        std::pair<bool, deque<AST*>> trans1(deque<AST*> start);

        bool semcheck1(deque<AST*> start);
        bool semcheck2(deque<AST*> start);
        void setsymbs(deque<AST*> lst);

        deque<AST*> process_ast_ll1();
        deque<AST*> process_ast_lalr1();
        std::set<std::string> get_alphabet();

    private:
        StartAST* _start;
        std::set<std::string> alphabet;
        std::set<std::string> nontermlist;
};

void processing_error(string err);
void astproc_err(std::string err, int lineno);

#endif