#ifndef ASTHANDLES_HPP
#define ASTHANDLES_HPP
#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <deque>
#include <set>
#include <map>
#include <iomanip>
#include <fstream>

#include "ast.hpp"
#include "astprocessing.hpp"
#include "../utilities/general_utils.hpp"

typedef std::pair<int, std::string> HandleDictPair;
typedef std::map<HandleDictPair, int> HandleDict;
typedef std::map<Literal*, std::vector<Literal*>> FollowSets;

using std::cerr,
      std::cout,
      std::map,
      std::pair,
      std::deque;


inline void handlefindingerror(std::string err) {
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
            return (_rule == comp.getRule()) && (_pos == comp.getPos());
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
        HandleFinder(deque<AST*> lst, std::set<std::string> _alph, string start_state) {
            _lst = lst;
            Rule* strt = new Rule(new Literal("S*", Tokens::RULE), new RuleList({new Literal(start_state, Tokens::RULE)}));
            AST_State frst = {};
            frst.push_back(strt);
            _states.push_back(frst);
            alphabet = _alph;
            _start_state = start_state;
        }
        
        /**
         * @brief Execute the Handle Finding Automata
         * 
         */
        void exec();

        /**
         * @brief Generate states using the Handle Finding Automata
         * 
         */
        void generate_states();

        /**
         * @brief Expand a state.
         * Done by looking at the symbol at the handle position and getting the RHS of all expressions that have the symbol at their LHS.
         * Then, new handles are created from these expressions, with handle position at 0, and added to the ASTState.
         * @param state AST_State to be expanded
         */
        void expand_state(AST_State &state);

        /**
         * @brief Print state-to-state transitions. No reduces included.
         * 
         */
        void print_transitions();

        /**
         * @brief Print all states being considered.
         * 
         */
        void print_states();

        void print_alt_grammar();

        void print_first_and_follow_sets();

        int getRuleCount() {return _lst.size();}
        int getStateCount() {return _states.size();}
        HandleDict getTransitions() {return transitions;}
        std::vector<AST_State> getStates() {return _states;}
        std::map<std::string, std::set<std::string>> getFollowSet() {return follow_set;}


        /**
         * @brief Backtrack from a state with a symbol, by finding all states that transition to it using the symbol
         * 
         * @param result_state The state being backtracked from.
         * @param trigger_lit The literal used to transition to the state being backtracked from.
         * @return vector<int>, a list of states that go to `result_state` using `trigger_lit`.
         */
        vector<int> backtrack_state(int result_state, string trigger_lit);

        void generate_table();
        std::vector<string> get_all_terms_and_nterms();
        void alt_grammar();
        bool create_first_set();
        bool create_follow_set();

    private:
        deque<AST*> _lst;
        string _start_state;
        HandleDict transitions;
        vector<AST_State> _states;
        vector<AltRule*> alt_grammar_list;
        std::set<std::string> alphabet;
        std::map<std::string, std::set<std::string>> first_set;
        std::map<std::string, std::set<std::string>> follow_set;
};


#endif