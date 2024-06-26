#ifndef AST_HPP
#define AST_HPP
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>

#include "../scanner/scanner.hpp"

using   std::cout, 
        std::cerr, 
        std::string, 
        std::vector, 
        std::make_pair, 
        std::deque;
//#define ASTPTR shared_ptr<AST>
//#define RLISTPTR shared_ptr<RuleList>
//#define LITPTR shared_ptr<Literal>


class AST;
class Literal;
class StartAST;
class RuleList;
class Rule;
class RegRule;
class OrExpr;
class ClosedExpr;
    class OptExpr;
    class RepExpr;
class EmptyAST;

class AST {
    public:
        AST() : _id("ast") {}
        bool isEmpty() {return _id == "empty";}
        string getId() {return _id;}
        virtual void print(int INDENT = 0) {
            cout << string(4*INDENT, ' ') << _id
                 << (((int)_children.size() > 0) 
                    ? ":" : "") << std::endl;
            for (auto child : _children)
                child->print(INDENT+1);
        }
        virtual deque<AST*> getChildren() {return _children;}
        virtual bool operator==(AST* o) {
            if (_id != o->getId()) return false;
            auto o_children = o->getChildren();
            if (_children.size() != o_children.size()) return false;
            for (int i = 0; i < _children.size(); ++i)
                if (!(_children[i] == o_children[i])) return false;
            return true;
        }
    protected:
        string _id;
        deque<AST*> _children;
};

class StartAST : public AST {
    public:
        StartAST() {_id = "start";}
        void add(AST* a) {
            _children.push_front(a);
        }
        deque<AST*> getChildren() {return _children;}
};

class Literal : public AST {
    public:
        Literal(string name, int tok = -1) {
            _id = (tok == Tokens::TOK) ? "tok" : "lit";
            _name = name;
            _tok = tok;
        }
        string getName() {return _name;}
        int getToken() {return _tok;}
        bool operator==(AST* o) override {
            if (_id != o->getId()) return false;
            auto t_o = (Literal*)o;
            return (_id == o->getId() && _name == t_o->getName());
        }
        virtual void print(int INDENT = 0) {
            cout << string(4*INDENT, ' ')
                 << _id << ", \"" 
                 << _name << "\""
                 << std::endl;
        }
    private:
        string _name;
        int _tok;
};

class RegRule : public AST {
    public:
        RegRule(string name, string regex) {
            _id = "regex";
            _name = name;
            _regex = regex;
        }

        RegRule(string name) {
            _id = "regex";
            _name = name;
            _regex = "";
        }

        void setRegex(string reg) {_regex = reg;}
        string getName() {return _name;}
        string getRegex() {return _regex;}

        virtual void print(int INDENT = 0) {
            cout << string(4 * INDENT, ' ')
                 << _name << " -> "
                 << _regex << std::endl;
        }

    private:
        string _name;
        string _regex;
};

class RuleList : public AST {
    public:
        RuleList() {
            _id = "list";
        }
        RuleList(AST* starter) {
            _id = "list";
            _children.push_back(starter);
        }
        RuleList(deque<AST*> list) {
            _id = "list";
            _children = list;
        }
        RuleList(RuleList &other) {
            _id = other.getId();
            _children = other.getChildren();
        }
        deque<AST*> getChildren() {return _children;}
        void addChild(AST* child) {
            _children.push_front(child);
        }
        AST* at(int index) {return _children.at(index);}
        AST* last() {return _children.back();}
        AST* first() {return _children.at(0);}
        bool isEmpty() {return _children.empty();}
        bool curr_is_or_node() {return !isEmpty() && last()->getId() == "orstmt";}
        virtual void print(int INDENT = 0) override {
            for (auto child : _children)
                child->print(INDENT);
        }
};

class OrExpr : public AST {
    public:
        OrExpr() {
            _id = "orstmt";
            _lhs = {};
            _rhs = {};
        }
        OrExpr(deque<AST*> arg1) {
            _id = "orstmt";
            _lhs = arg1;
            _rhs = {};
        }
        OrExpr(deque<AST*> arg1, deque<AST*> arg2) {
            _id = "orstmt";
            _lhs = arg1;
            _rhs = arg2;
        }
        void add(AST* a) {_lhs.push_front(a);}
        deque<AST*> getLeft() {return _lhs;}
        deque<AST*> getRight() {return _rhs;}
        void swap() {
            if (_rhs.empty()) {
                _rhs = _lhs;
                _lhs = {};
            } else {
                _rhs = {new OrExpr(_lhs, _rhs)};
                _lhs = {};
            }
        }
        bool emptyleft() {return _lhs.empty();}
        bool emptyright() {return _rhs.empty();}
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << _id << ":" 
                 << std::endl
                 << string(4*(INDENT+1), ' ')
                 << "left: "
                 << std::endl;
            for (auto left : _lhs)
                left->print(INDENT+2);
            cout << string(4*(INDENT+1), ' ')
                 << "right: "
                 << std::endl;
            for (auto rght : _rhs)
                rght->print(INDENT+2);
        }
    private:
        deque<AST*> _lhs;
        deque<AST*> _rhs;
};

class Rule : public AST {
    public:
        Rule(Literal* ast1, RuleList* rlist1) {
            _id   = "rule";
            _lhs  = ast1;
            _rhs  = rlist1;
            _folw = {};
        }
        
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << "rule: "
                 << _lhs->getName() << " [";
            for (int i = 0; i < _folw.size(); ++i)
                cout << _folw[i]
                     << (i == _folw.size()-1 ? "" : ", ");
            cout << "]\n";
            for (auto term : _rhs->getChildren())
                term->print(INDENT+1);
        }

        Literal* getLeft() {return _lhs;}
        RuleList* getRight() {return _rhs;}
        
        void addFollow(string str) {
            _folw.push_back(str);
        }

        std::deque<std::string> getFollow() {
            return _folw;
        }

    private:
        Literal* _lhs;
        RuleList* _rhs;
        deque<string> _folw;
};

static inline bool 
rulecmp(Rule* a, Rule* b)
{
    int ind   = 0;
    auto lita = a->getLeft();
    auto litb = b->getLeft();
    auto lsta = a->getRight()->getChildren();
    auto lstb = b->getRight()->getChildren();

    if (lita->getId() != litb->getId()) return false;
    if (lita->getName() != litb->getName()) return false;
    if (lsta.size() != lstb.size()) return false;

    for (ind = 0; ind < lsta.size(); ++ind) {
        auto itemA = lsta[ind];
        auto itemB = lstb[ind];

        if (itemA->getId() != itemB->getId()) return false;

        if (itemA->getId() == "lit" 
         || itemA->getId() == "tok") {
            auto stra = ((Literal*)itemA)->getName();
            auto strb = ((Literal*)itemB)->getName();
            if (stra != strb) return false;
        } else if (itemA->getId() != "empty") return false;
    }

    return true;
}

class ClosedExpr : public AST {
    public:
        ClosedExpr(RuleList* inner) : _in(inner) {
            _id = "closed-expr";
        }
        RuleList* getExpr() {return _in;}
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << _id << ":" 
                 << std::endl;
            _in->print(INDENT+1);
        }
    protected:
        RuleList* _in;
};

class OptExpr : public ClosedExpr {
    public:
        OptExpr(RuleList* inner) : ClosedExpr(inner) {
            _id = "opt-expr";
        }
};

class RepExpr : public ClosedExpr {
    public:
        RepExpr(RuleList* inner) : ClosedExpr(inner) {
            _id = "rep-expr";
        }
};

class EmptyAST : public AST {
    public:
        EmptyAST() {_id = "empty";}
};

#endif 
