#ifndef AST_HPP
#define AST_HPP
#pragma once

using std::cout, std::cerr, std::string, std::vector, std::make_pair, std::shared_ptr;
//#define ASTPTR shared_ptr<AST>
//#define RLISTPTR shared_ptr<RuleList>
//#define LITPTR shared_ptr<Literal>


class AST;
class Literal;
class StartAST;
class RuleList;
class Rule;
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
    protected:
        string _id;
        vector<AST*> _children;
};

class StartAST : public AST {
    public:
        StartAST() {_id = "start";}
        void add(AST* a) {_children.push_back(a);}
};

class Literal : public AST {
    public:
        Literal(string name, int tok = -1) {
            _id = "lit";
            _name = name;
            _tok = tok;
        }
        string getName() {return _name;}
        int getToken() {return _tok;}
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

class RuleList : public AST {
    public:
        RuleList() {
            _id = "list";
        }
        RuleList(AST* starter) {
            _id = "list";
            _children.push_back(starter);
        }
        vector<AST*> getChildren() {return _children;}
        void addChild(AST* child) {
            _children.push_back(child);
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
        OrExpr(RuleList* arg1) {
            _id = "orstmt";
            _rhs = arg1;
            _lhs = new RuleList();
        }
        OrExpr(RuleList* arg1, RuleList* arg2) {
            _id = "orstmt";
            _lhs = arg1;
            _rhs = arg2;
        }
        void addLeft(AST* a) {_lhs->addChild(a);}
        void addRight(AST* a) {_rhs->addChild(a);}
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << _id << ":" 
                 << std::endl
                 << string(4*(INDENT+1), ' ')
                 << "left: "
                 << std::endl;
            for (auto left : _lhs->getChildren())
                left->print(INDENT+2);
            cout << string(4*(INDENT+1), ' ')
                 << "right: "
                 << std::endl;
            for (auto rght : _rhs->getChildren())
                rght->print(INDENT+2);
        }
    private:
        RuleList* _lhs;
        RuleList* _rhs;
};

class Rule : public AST {
    public:
        Rule(Literal* ast1, RuleList* rlist1) {
            _id = "rule";
            _lhs = ast1;
            _rhs = rlist1;
        }
        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ') 
                 << "rule: "
                 << _lhs->getName() << std::endl;
            for (auto term : _rhs->getChildren())
                term->print(INDENT+1);
        }
    private:
        Literal* _lhs;
        RuleList* _rhs;
};

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