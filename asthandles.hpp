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
#include <iomanip>

typedef std::pair<int, std::string> HandleDictPair;
typedef std::map<HandleDictPair, int> HandleDict;
typedef std::map<Literal*, std::vector<Literal*>> FollowSets;

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

        /**
         * @brief Construct a new ASTHandle object
         * 
         * @param rule Rule stored within ASTHandle object
         * @param pos Position of the handle in the ASTHandle object
         */
        ASTHandle(Rule* rule, int pos = 0) {
            _rule = rule;
            auto rright = _rule->getRight()->getChildren();
            bool cond = rright.size() == 1 && rright[0]->getId() == "empty";
            _pos = cond ? rright.size() : pos;
            if (cond) _isempt = true;
        }

        /**
         * @brief Move the handle once rightwards
         * 
         */
        void bump() {
            auto r = _rule->getRight()->getChildren();
            if (_pos < r.size()) ++_pos;
        }

        /**
         * @brief Check if the handle is empty (containing only an empty rule)
         * 
         * @return true, if the handle is empty
         * @return false otherwise
         */
        bool empty() {return _isempt;}

        /**
         * @brief Check if the handle position is at the very end of the rule
         * 
         * @return true, if handle position is at the end of the rule,
         * @return false otherwise
         */
        bool closed() {
            auto r = _rule->getRight()->getChildren();
            return (int)r.size() == _pos;
        }

        /**
         * @brief Return the symbol immediately to the right of the handle 
         * 
         * @return Pointer to AST element to the right of the handle,
         * @return nullptr if no such element exists
         */
        AST* handlesymb() {
            auto r = _rule->getRight()->getChildren();
            if (_pos >= (int)r.size()) {
                return nullptr;
            } return r.at(_pos);
        }

        /**
         * @brief Get the Rule object stored within the handle
         * 
         * @return Pointer to Rule object within the handle
         */
        Rule* getRule() {return _rule;}

        /**
         * @brief Get the position of the handle
         * 
         * @return Position of the handle
         */
        int getPos() {return _pos;}

        /**
         * @brief Check if two handles are equal
         * 
         * @param comp Handle with which to compare equality
         * @return true if both the rule and the handle positions are the same,
         * @return false otherwise
         */
        bool iseq(ASTHandle &comp) {
            return rule_eq(_rule, comp.getRule()) && (_pos == comp.getPos());
        }

        /**
         * @brief Print out handles. Note that `e` is used to represent empty rules.
         * 
         */
        void print() {
            //cout << "handle at pos " << _pos << std::endl;
            //_rule->print();
            
            auto lhs = _rule->getLeft();
            auto rhs = _rule->getRight()->getChildren();

            std::string resultstr;
            resultstr += lhs->getName() + " -> ";
            for (int i = 0; i < rhs.size(); ++i) {
                if (i == _pos) resultstr += ". ";
                auto i_rhs = (Literal*)rhs[i];
                auto i_rhs_type = rhs[i]->getId();
                if (i_rhs_type == "empty")
                    resultstr += "e ";
                else {
                    auto i_rhs = (Literal*)rhs[i];
                    if (i_rhs_type == "lit")
                        resultstr += i_rhs->getName() + " ";
                    else resultstr += "\"" + i_rhs->getName() + "\" ";
                }
            } if (_pos >= rhs.size()) resultstr += ".";
            
            std::cout << resultstr << std::endl;
        }
    private:
        Rule* _rule;                            // Rule in which handle is stored
        int _pos;                               // Position of handle within the rule
        bool _isempt = false;                   // Boolean checking whether rule is empty
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
        std::string getName() {
            if (_id == "empty") return "";
            auto l_id = (Literal*)_ref;
            if (_id == "lit")
                return l_id->getName() + "@" + std::to_string(_state);
            else return "\"" + l_id->getName() + "\"@" + std::to_string(_state);
        }
        bool operator==(AltAST* other) {
            auto o_id = other->getId();
            if (_id != o_id) return false;
            if (_state != other->getState()) return false;
            
            if (_id == "empty") return true;

            auto i_name = ((Literal*)_ref)->getName();
            auto o_name = ((Literal*)other->getAST())->getName();
            return i_name == o_name;
        }
        virtual void print(int INDENT = 0) override {
            cout << string(INDENT*4, ' ') << "#" << _state << ", ";
            _ref->print();
        }
    private:
        AST* _ref;
        int _state;
};

class AltRuleList : public AST {
    public:
        AltRuleList() {
            _id = "altlist";
        }
        AltRuleList(AltAST* starter) {
            _id = "list";
            alt_children.push_back(starter);
        }
        AltRuleList(deque<AltAST*> list) {
            _id = "list";
            alt_children = list;
        }
        deque<AltAST*> getAltChildren() {return alt_children;}
        void addChild(AltAST* child) {
            alt_children.push_back(child);
        }
        AltAST* at(int index) {return alt_children.at(index);}
        AltAST* last() {return alt_children.back();}
        AltAST* first() {return alt_children.at(0);}
        int size() {return alt_children.size();}
        bool isEmpty() {return alt_children.empty();}
        bool curr_is_or_node() {return !isEmpty() && last()->getId() == "orstmt";}
        virtual void print(int INDENT = 0) override {
            for (auto child : alt_children)
                child->print(INDENT);
        }
    private:
        deque<AltAST*> alt_children;
};

class AltRule : public AST {
    public:
        AltRule(AltAST* ast1, AltRuleList* rlist1) {
            _id = "rule";
            _lhs = ast1;
            _rhs = rlist1;
        }
        AltRuleList* getRight() {return _rhs;}
        AltAST* getLeft() {return _lhs;}
        virtual void print(int INDENT = 0) override {
            std::string resultstr;
            resultstr += _lhs->getName() + " -> ";

            for (int i = 0; i < _rhs->size(); ++i) {
                auto i_rhs = _rhs->at(i);
                auto i_rhs_type = _rhs->at(i)->getId();
                if (i_rhs_type == "empty")
                    resultstr += "e ";
                else {
                    auto dk = (Literal*)(i_rhs->getAST());
                    resultstr += ((i_rhs_type == "lit") ? dk->getName() : ("\"" + dk->getName() + "\""));
                    resultstr += "@" + std::to_string(i_rhs->getState()) + " ";
                }
            }
            
            std::cout << resultstr << std::endl;
        }
    
    private:
        AltAST* _lhs;
        AltRuleList* _rhs;
};

class HandleFinder {
    public:

        /**
         * @brief Construct a new Handle Finder object
         * 
         * @param lst List of Rule objects from which to form ASTHandle objects
         */
        HandleFinder(deque<AST*> lst) {
            _lst = lst;
            Rule* strt = new Rule(new Literal("S*", Tokens::RULE), new RuleList({new Literal("Start", Tokens::RULE)}));
            AST_State frst = {};
            frst.push_back(strt);
            states.push_back(frst);
        }
        
        /**
         * @brief Execute the Handle Finding Automata
         * 
         */
        void exec() {
            int i = 0;
            expand_state(states[0]);
            while (1) {
                auto state = states[i++];

                // create transition states from all states for each symbol in the alphabet
                for (auto e : alphabet) {
                    // new state
                    AST_State y = {};

                    /**
                     * Add handles to new state formed by bumping the handles of the last state up by one.
                     */

                    for (auto s : state) {
                        // get symbol at position of handle
                        auto kr = s.handlesymb();
                        // ignore if the handle is at the end
                        if (!kr) continue;
                        
                        // predict string representation of symbol
                        auto k = (kr->getId() == "empty") 
                                ? "empty" 
                                : ((Literal*)kr)->getName();
                        // clone of current alphabet symbol being considered
                        auto etrue = e;

                        // remove terminal "#" prefix for prediction
                        if (etrue.at(0) == '#') 
                            etrue.erase(etrue.begin());
                        
                        // if the symbol at the handle is the alphabet symbol 'e':
                        if (k == e && kr->getId() == "lit" || k == etrue && kr->getId() == "tok") {
                            // push back new handle with handle position bumped up one
                            auto b = s;
                            b.bump();
                            y.push_back(b);
                        }
                    }

                    // expand resultant y state
                    // done by getting resultant rules from the new handle positions
                    expand_state(y);
                    
                    /** 
                     * check if state N already exists using the following algorithm:
                     *   0. if S has no more next state, then unequal
                     *   1. get next state S from the array of states
                     *   2. if S's size is unequal to N's size, go to line 0
                     *   3. get AST_State at index i in state S
                     *   4. for all items in AST_State, if S[i] is not N[i], go to line 0
                     *   5. if i is not the last state, go to line 3; else, S and N are equal
                     * old_state is set to -1 if the state is completely new, and as the identical state's number otherwise
                    */
                    bool state_exists = false;         // boolean indicating whether state exists
                    int old_state = -1;                // int preset to -1, changes to the state number of an identical state if it exists
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

                    // insertion of transition
                    // if state was new, the new state `y` is also inserted
                    if ((int)y.size() > 0) {
                        int arg2 = (state_exists) 
                                ? old_state 
                                : (int)states.size();
                        HandleDictPair arg1 = std::make_pair(i-1, e);
                        transitions.emplace(std::make_pair(arg1, arg2));
                        if (!state_exists) states.push_back(y);
                    }
                }
                
                // break only if i passes the bounds
                // since the loop actively adds to the size of the state list
                // this is only true if a loop iteration goes without any new states being made, among other things...
                if (i == states.size())
                    break;
            }

            print_states();
            print_transitions();
        }

        /**
         * @brief Expand a state.
         * Done by looking at the symbol at the handle position and getting the RHS of all expressions that have the symbol at their LHS.
         * Then, new handles are created from these expressions, with handle position at 0, and added to the ASTState.
         * @param state AST_State to be expanded
         */
        void expand_state(AST_State &state) {
            int i = 0;
            std::set<string> alrseen;
            //cout << "expanding state of size "
            //     << state.size() << std::endl;
            while (1) {
                if (i == state.size())
                    break;
                
                ASTHandle handle = state[i++];
                auto symb = handle.handlesymb();
                if (!symb)
                    continue;

                // check if symb is a Rule
                // if already handled the symb, ignore
                // else add all instances of states in _lst that begin with that symbol to states, 
                // then add its name to alrseen, so that it is not worked on again
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
        }

        /**
         * @brief Print state-to-state transitions. No reduces included.
         * 
         */
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

        /**
         * @brief Print all states being considered.
         * 
         */
        void print_states() {
            cout << "printing states\n";
            for (int i = 0; i < states.size(); i++) {
                cout << "state " << i << std::endl;
                for (auto d : states[i]) d.print();
                cout << std::endl;
            }
            cout << std::endl;
        }

        void print_alt_grammar() {
            cout << "printing alt grammar\n";
            for (auto a : alt_grammar_list)
                a->print();
            cout << std::endl;
        }

        void print_first_and_follow_sets() {
            std::vector<string> namestrings = {"name"}, firstsetstrings = {"first"}, followsetstrings = {"follow"};

            for (auto fst : first_set) {
                auto fstkey = fst.first;
                namestrings.push_back(fstkey);
                std::string fs, fl;
                for (auto fstel : fst.second) {
                    fs += fstel + ", ";
                }
                if (!fs.empty()) fs = fs.substr(0, fs.size()-2);
                for (auto flwel : follow_set[fst.first]) {
                    fl += flwel + ", ";
                }
                if (!fl.empty()) fl = fl.substr(0, fl.size()-2);
                firstsetstrings.push_back(fs);
                followsetstrings.push_back(fl);
            }

            int maxname = 0, maxfirst = 0, maxfollow = 0;
            for (auto x : namestrings) if (x.length() > maxname) maxname = x.length();
            for (auto x : firstsetstrings) if (x.length() > maxfirst) maxfirst = x.length();
            for (auto x : followsetstrings) if (x.length() > maxfollow) maxfollow = x.length();

            auto filler = [](int number, char ch) {
                std::string ret;
                for (int i = 0; i < number; ++i)
                    ret += ch;
                return ret;
            };

            std::cout << "+-" << filler(maxname, '-');
            std::cout << "-+-" << filler(maxfirst, '-');
            std::cout << "-+-" << filler(maxfollow, '-') << "-+\n";

            for (int i = 0; i < namestrings.size(); ++i) {
                std::cout << "| " << std::left << std::setw(maxname) << namestrings[i];
                std::cout << " | " << std::left << std::setw(maxfirst) << firstsetstrings[i];
                std::cout << " | " << std::left << std::setw(maxfollow) << followsetstrings[i] << " |\n";
                std::cout << "+-" << filler(maxname, '-');
                std::cout << "-+-" << filler(maxfirst, '-');
                std::cout << "-+-" << filler(maxfollow, '-') << "-+\n";
            }


            // ┌───────────────┬─────────────┬────────────┬────────────┬────────────┐
            // ├───────────────┼─────────────┼────────────┼────────────┼────────────┤
            // └───────────────┴─────────────┴────────────┴────────────┴────────────┘
            // │││││
        }

        /**
         * @brief Backtrack from a state with a symbol, by finding all states that transition to it using the symbol
         * 
         * @param result_state The state being backtracked from.
         * @param trigger_lit The literal used to transition to the state being backtracked from.
         * @return vector<int>, a list of states that go to `result_state` using `trigger_lit`.
         */
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
            if (states.size() <= 0) 
                return;

            for (int state = 0; state < states.size(); state++) {
                for (auto handle : states[state]) {
                    auto handlerule = handle.getRule();
                    // don't handle start states
                    if (handlerule->getLeft()->getName() == "S*") continue;
                    // don't handle rules where the handle is not at the front, unless it's an empty rule
                    if (handle.getPos() != 0 && !handle.empty()) continue;
                    // initialize lhs and rhs
                    auto altlhs = new AltAST(handlerule->getLeft(), state);
                    auto altrhs = new AltRuleList();
                    // first state is current state
                    auto curstate = state;
                    // loop through the Literals in rhs
                    auto handlerhschildren = handlerule->getRight()->getChildren();
                    for (auto i = 0; i < handlerhschildren.size(); ++i) {
                        auto plainrhs = handlerhschildren[i];
                        altrhs->addChild(new AltAST(plainrhs, curstate));
                        if (handle.empty()) break;
                        auto plainrhs_name = (plainrhs->getId() == "lit" ? "" : "#") + ((Literal*)plainrhs)->getName();
                        if (i != handlerhschildren.size() - 1 
                        && transitions.find(HandleDictPair(curstate, plainrhs_name)) != transitions.end()) {
                            curstate = transitions[HandleDictPair(curstate, plainrhs_name)];
                        }
                        else break;
                    }
                    alt_grammar_list.push_back(new AltRule(altlhs, altrhs));
                }
            }

            print_alt_grammar();
        }

        bool create_first_set() {
            // compute first sets with alt grammar
            // a simple loop; loop until break
            // boolean indicating whether a change occured
            bool no_change = true;

            for (int i = 0; i < alt_grammar_list.size(); ++i) {
                auto item = alt_grammar_list[i]->getLeft()->getName();
                auto initsize = first_set[item].size();

                auto rhs_items = alt_grammar_list[i]->getRight()->getAltChildren();
                auto rhs_first = rhs_items.at(0);

                if (rhs_first->getId() == "lit") {
                    if (first_set.find(rhs_first->getName()) != first_set.end())
                        for (auto fsvals : first_set[rhs_first->getName()])
                            first_set[item].insert(fsvals);
                }
                else
                if (rhs_first->getId() == "tok") {
                    first_set[item].insert(rhs_first->getName());
                }
                else {
                    if (rhs_first->getId() == "empty")
                        if (follow_set.find(item) != follow_set.end())
                            for (auto fsvals : follow_set[item])
                                first_set[item].insert(fsvals);
                }
                auto endsize = first_set[item].size();
                no_change = no_change && (initsize == endsize);
            }
            
            return no_change;
        }

        bool create_follow_set() {
            // compute follow sets with alt grammar
            // a simple loop; loop until break
            bool no_change = true;

            for (int i = 0; i < alt_grammar_list.size(); ++i) {
                auto item = alt_grammar_list[i]->getLeft()->getName();
                auto initsize = follow_set[item].size();
                if (item == "Start@0") follow_set[item].insert("$");

                for (auto x : alt_grammar_list) {
                    auto rhs_items = x->getRight()->getAltChildren();
                    for (int i = 0; i < rhs_items.size(); ++i) {
                        auto rhs_item = rhs_items[i]->getName();
                        if (item == rhs_item) {
                            if (i != rhs_items.size()-1) {
                                auto nxt = rhs_items[i+1];
                                if (nxt->getId() == "lit") {
                                    if (first_set.find(nxt->getName()) != first_set.end()) {
                                        for (auto fsvals : first_set[nxt->getName()])
                                            follow_set[item].insert(fsvals);
                                    }
                                }
                                else
                                if (nxt->getId() == "tok") {
                                    follow_set[item].insert(nxt->getName());
                                }
                            }
                            else {
                                auto lhs = x->getLeft()->getName();
                                if (follow_set.find(lhs) != follow_set.end()) {
                                    for (auto fsvals : follow_set[lhs])
                                        follow_set[item].insert(fsvals);
                                }
                            }
                        }
                    }
                }

                auto endsize = follow_set[item].size();
                no_change = no_change && (initsize == endsize);
            }

            return no_change;
        }

        void generate_table() {
            // initialize alt_grammar
            alt_grammar();
            // load first and follow sets
            for (int i = 0; i < alt_grammar_list.size(); ++i) {
                auto item = alt_grammar_list[i]->getLeft();
                auto str = ((Literal*)(item->getAST()))->getName() + "@" + std::to_string(item->getState());
                if (first_set.find(str) == first_set.end())
                    first_set[str] = std::set<std::string>{};
                if (follow_set.find(str) == follow_set.end())
                    follow_set[str] = std::set<std::string>{};
            }
            // finalize first and follow sets
            while (!create_first_set() || !create_follow_set());
            // i kind of want to see the first and follow sets
            print_first_and_follow_sets();
        }

    private:
        deque<AST*> _lst;
        HandleDict transitions;
        vector<AST_State> states;
        vector<AltRule*> alt_grammar_list;
        std::map<std::string, std::set<std::string>> first_set;
        std::map<std::string, std::set<std::string>> follow_set;
};


#endif