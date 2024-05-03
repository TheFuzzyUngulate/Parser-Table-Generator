#ifndef ASTPROCESSING_HPP
#define ASTPROCESSING_HPP
#pragma once

#include "ast.hpp"
#include "../utilities/general_utils.hpp"
#include <set>
#include <map>
#include <string>

typedef std::map<std::string, std::set<std::string>> FollowSetMap;

class ASTProcessor {
    public:
        ASTProcessor(StartAST* start, bool show) {
            _start = start;
            _showProc = show;
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

        bool semcheck1(deque<AST*> start, string start_state);
        bool semcheck2(deque<AST*> start);
        void setsymbs(deque<AST*> lst);

        deque<AST*> process_ast_ll1();
        deque<AST*> process_ast_lalr1(string start_state);
        std::set<std::string> get_alphabet();

    private:
        bool _showProc;
        StartAST* _start;
        std::set<std::string> alphabet;
        std::set<std::string> nontermlist;
};

void processing_error(string err);
void astproc_err(std::string err, int lineno);

#endif