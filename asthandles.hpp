#ifndef ASTHANDLES_HPP
#define ASTHANDLES_HPP
#pragma once

#include "astprocessing.hpp"
#include "ast.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <deque>
#include <set>
#include <map>

typedef std::pair<int, std::string> HandleDictPair;
typedef std::map<HandleDictPair, int> HandleDict;

using std::cerr,
      std::cout,
      std::map,
      std::pair;

class ASTHandle {
    public:
        ASTHandle(Rule* rule, int pos = 0) {
            _rule = rule;
            _pos = pos;
        }
        void bump() {
            auto r = _rule->getRight()->getChildren();
            if (_pos < r.size()) ++_pos;
        }
        bool closed() {
            auto r = _rule->getRight()->getChildren();
            return (int)r.size() == _pos;
        }
        AST* handlesymb() {
            auto r = _rule->getRight()->getChildren();
            if (_pos >= (int)r.size()) {
                return nullptr;
            } return r.at(_pos);
        }
        Rule* getRule() {return _rule;}
        int getPos() {return _pos;}
        bool iseq(ASTHandle &comp) {
            return rule_eq(_rule, comp.getRule()) && (_pos == comp.getPos());
        }
        void print() {
            cout << "handle at pos " << _pos << std::endl;
            _rule->print();
            cout << std::endl;
        }
    private:
        Rule* _rule;
        int _pos;
};
typedef std::deque<ASTHandle> AST_State;

class HandleFinder {
    public:
        HandleFinder(deque<AST*> lst) {
            _lst = lst;
            Rule* strt = new Rule(new Literal("S*", Tokens::RULE), new RuleList({new Literal("Start", Tokens::RULE)}));
            AST_State frst = {};
            frst.push_back(strt);
            states.push_back(frst);
        }
        
        void exec() {
            int i = 0;
            while (1) {
                auto state = states[i++];
                expand_state(state);
                for (auto e : alphabet) {
                    AST_State y = {};

                    // creating new state
                    //cout << "creating new state...\n";
                    for (auto s : state) {
                        auto kr = s.handlesymb();
                        if (!kr) continue;
                        auto k = (kr->getId() == "empty") 
                                ? "empty" 
                                : ((Literal*)kr)->getName();
                        auto etrue = e;
                        if (etrue.at(0) == '#') 
                            etrue.erase(etrue.begin());
                        if (k == e && kr->getId() == "lit"
                            || k == etrue && kr->getId() == "tok") {
                            auto b = s;
                            b.bump();
                            y.push_back(b);
                        }
                    }
                    
                    // state existence search
                    //cout << "checking if state already exists...\n";
                    bool state_exists = false;
                    int old_state = -1;
                    for (int g = 0; g < states.size(); g++) {
                        auto s = states[g];
                        if (s.size() != y.size())
                            continue;
                        bool eq_state = true;
                        for (int j = 0; j < s.size(); ++j) {
                            if (!s.at(j).iseq(y.at(j))) {
                                eq_state = false;
                                break;
                            }
                        }
                        if (eq_state) {
                            state_exists = true;
                            old_state = i;
                            break;
                        }
                    }

                    // insertion of new state (and transition)
                    //cout << "inserting new state...\n";
                    if ((int)y.size() > 0) {
                        int arg2 = (state_exists) 
                                ? old_state 
                                : (int)states.size();
                        HandleDictPair arg1 = std::make_pair(i-1, e);
                        transitions.emplace(std::make_pair(arg1, arg2));
                        if (!state_exists) states.push_back(y);
                    }
                }
                //std::cout << "finished working on state" 
                         // << i-1 << std::endl << std::endl;
                if (i == states.size())
                    break;
            }
        }

        void expand_state(AST_State &state) {
            int i = 0;
            std::set<string> alrseen;
            //cout << "expanding state of size "
            //     << state.size() << std::endl;
            while (1) {
                if (i == state.size())
                    break;
                //cout << "first loop forward\n";
                ASTHandle handle = state[i++];
                auto symb = handle.handlesymb();
                if (!symb)
                    continue;
                // check if symb is a Rule
                // if its name is in alrseen, ignore
                // else add all instances of states in _lst that begin with that symbol to states, 
                // then add its name to alrseen
                if (symb->getId() == "lit") {
                    string lname = ((Literal*)symb)->getName();
                    if (alrseen.find(lname) == alrseen.end()) {
                        for (auto k : _lst) {
                            Rule *k_rule = (Rule*)k;
                            if (k_rule->getLeft()->getName() == lname) {
                                state.push_back(ASTHandle(k_rule));
                            }
                        }
                        alrseen.insert(lname);
                    }
                }
            }
            //cout << "ending state expansion...\n\n";
        }

        void print_transitions() {
            cout << "printing transitions...\n";
            for (auto x : transitions) {
                cout << "(" 
                     << x.first.first << ", "
                     << x.first.second
                     << ") => "
                     << x.second
                     << std::endl;
            } cout << std::endl;
        }

        void print_states() {
            cout << "printing states\n";
            for (int i = 0; i < states.size(); i++) {
                cout << "state " << i << std::endl;
                expand_state(states[i]);
                for (auto d : states[i]) d.print();
                cout << std::endl;
            }
            cout << std::endl;
        }

    private:
        deque<AST*> _lst;
        vector<AST_State> states;
        HandleDict transitions;
};


#endif