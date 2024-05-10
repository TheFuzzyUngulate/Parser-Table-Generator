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
        ASTProcessor(deque<Rule*> start, bool show) {
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
        deque<Rule*> trans6(deque<Rule*> start);
        std::pair<bool, deque<Rule*>> trans5(deque<Rule*> start);
        std::pair<bool, deque<Rule*>> trans4(deque<Rule*> start);
        std::pair<bool, deque<Rule*>> trans3(deque<Rule*> start);
        std::pair<bool, deque<Rule*>> trans2(deque<Rule*> start);
        std::pair<bool, deque<Rule*>> trans1(deque<Rule*> start);

        bool semcheck1(deque<Rule*> start, string start_state);
        bool semcheck2(deque<Rule*> start);
        void setsymbs(deque<Rule*> lst);

        deque<Rule*> process_ast_ll1();
        deque<Rule*> process_ast_lalr1(string start_state);
        std::set<std::string> get_alphabet();

    private:
        bool _showProc;
        deque<Rule*> _start;
        std::set<std::string> alphabet;
        std::set<std::string> nontermlist;
};

void processing_error(string err);
void astproc_err(std::string err, int lineno);

#endif