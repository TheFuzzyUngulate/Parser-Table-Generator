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


void handlefindingerror(std::string err) {
    cout << "handle finding error: " << err << std::endl;
    exit(-1);
}

class ASTHandle {
    public:
        ASTHandle(Rule* rule, int pos = 0) {
            _rule = rule;
            auto rright = _rule->getRight()->getChildren();
            bool cond = rright.size() == 1 && rright[0]->getId() == "empty";
            _pos = cond ? rright.size() : pos;
            if (cond) _isempt = true;
        }
        void bump() {
            auto r = _rule->getRight()->getChildren();
            if (_pos < r.size()) ++_pos;
        }
        bool empty() {return _isempt;}
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
        }
    private:
        Rule* _rule;
        int _pos;
        bool _isempt = false;
};

typedef std::deque<ASTHandle> AST_State;

class AltAST : public AST {
    public:
        AltAST(AST* ref, int state) {
            _ref = ref;
            _state = state;
            _id = ref->getId();
        }
        int getState() {return _state;}
        AST* getAST() {return _ref;}
        virtual void print(int INDENT = 0) override {
            cout << string(INDENT*4, ' ') << "#" << _state << ", ";
            _ref->print();
        }
    private:
        AST* _ref;
        int _state;
};

class AltRule : public AST {
    public:
        AltRule(AltAST* ast1, RuleList* rlist1) {
            _id = "rule";
            _lhs = ast1;
            _rhs = rlist1;
        }
        RuleList* getRight() {return _rhs;}
        AltAST* getLeft() {return _lhs;}
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << "rule: ";
            _lhs->print();
            for (auto term : _rhs->getChildren())
                term->print(INDENT+1);
        }
    
    private:
        AltAST* _lhs;
        RuleList* _rhs;
};

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
            expand_state(states[0]);
            while (1) {
                auto state = states[i++];
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

                    // expand y state
                    expand_state(y);
                    
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
                            old_state = g;
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

            print_states();
            print_transitions();
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
                //expand_state(states[i]);
                for (auto d : states[i]) d.print();
                cout << std::endl;
            }
            cout << std::endl;
        }

        vector<int> backtrack_state(int result_state, string trigger_lit) {
            // find int "A" such that (A, trigger_lit) = result_state
            // so first, find all things whose result is result_state
            // note that this presumes trigger_lit has the "#" delimiter when suitable
            vector<int> res = {};
            for (auto trans : transitions) {
                if (trans.second == result_state && trans.first.second == trigger_lit) {
                    res.push_back(trans.first.first);
                }
            } 
            if (res.empty())
                handlefindingerror("unsuccessful backtrack on " + trigger_lit + "  in state " + std::to_string(result_state));
            return res;
        }

        void alt_grammar() {
            if (states.size() <= 0) return;
            vector<AltRule*> alt_rules;

            // handle non-empties, then empties ig..
            for (int i = 0; i < states.size(); i++) {
                for (auto handle : states[i]) {
                    if (!handle.closed()) continue;
                    auto rule = handle.getRule();

                    if (rule->getLeft()->getName() == "S*") continue;
                    auto rightlist = rule->getRight()->getChildren();

                    int cur_state = i;
                    if (handle.empty()) {
                        auto left = new AltAST(rule->getLeft(), i);
                        alt_rules.push_back(new AltRule(left, rule->getRight()));
                    }
                    else {
                        auto reslist = get_altgram_rhs(rightlist, cur_state);
                        for (auto r : reslist) {
                            int first = r.first;
                            auto left = new AltAST(rule->getLeft(), first);
                            alt_rules.push_back(new AltRule(left, new RuleList(r.second)));
                        }
                    }
                }
            }

            alt_grammar_list = alt_rules;
        }

    private:
        vector<std::pair<int, deque<AST*>>> 
        get_altgram_rhs(deque<AST*> rhs, int cur_state) {
            // create returning vector
            vector<std::pair<int, deque<AST*>>> res_vec = {};
            // get last item in rhs (last)
            if (rhs.empty()) {
                std::deque<AST*> ro = {};
                res_vec.push_back(std::make_pair(cur_state, ro));
                return res_vec;
            }
            auto item_lst = (Literal*)rhs.back();
            // create the string for it (curstr)
            auto curstr = ((item_lst->getId() == "tok") ? "#" : "") + item_lst->getName();
            // get all possible states "x" such that (x, curstr) => cur_state
            auto state_lst = backtrack_state(cur_state, curstr);
            // for each possible "x":
            for (auto x : state_lst) {
                // recurse on (rhs.pop(), x)
                deque<AST*> popped_rhs(rhs.begin(), rhs.end()-1);
                auto curret = get_altgram_rhs(popped_rhs, x);
                // push back strings curstr + to_string(x)
                if (curret.empty()) {
                    std::deque<AST*> ro = {};
                    res_vec.push_back(std::make_pair(x, ro));
                }
                else
                for (auto y : curret) {
                    //y.second.push_back(new Literal(item_lst->getName() + std::to_string(x), item_lst->getToken()));
                    y.second.push_back(new AltAST(item_lst, x));
                    res_vec.push_back(y);
                }
            }
            // return value
            return res_vec;
        }

        deque<AST*> _lst;
        HandleDict transitions;
        vector<AST_State> states;
        vector<AltRule*> alt_grammar_list;
};


#endif