#ifndef AST_HPP
#define AST_HPP
#pragma once

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

class ast {
    public:
        ast(string type = "ast", int t = -1) {
            _type = type;
            _tok = t;
        }
        virtual void print(int INDENT = 0) {
            cout << string(4*INDENT, ' ')
                 << _type 
                 << ((_tok != -1) 
                    ? ", "+string(tokname(_tok)) 
                    : "")
                 << std::endl;
            for (int i = _chlds.size()-1; i >= 0; --i)
                _chlds[i]->print(INDENT+1);
        }
        virtual string get_type() {return _type;}
        int get_tok() {return _tok;}
        virtual vector<ast*> getchlds() {return _chlds;}
        virtual void setchlds(vector<ast*> ch) {_chlds = ch;}
        virtual void add(ast* added) {_chlds.push_back(added);}

    protected:
        vector<ast*> _chlds;
        string _type;
        int _tok;
};

class ast_empty : public ast {
    public:
        ast_empty() : ast("empty", Tokens::EMPTY) {}
};

class lit : public ast {
    public:
        lit(int t, string lex) : ast("lit", t) {
            _lex = lex;
        }

        string get_lex() {return _lex;}

        virtual void print(int INDENT = 0) override {
            cout << string(4*INDENT, ' ')
                 << _type 
                 << ((_tok != -1) 
                    ? ", "+string(tokname(_tok)) 
                    : "")
                 << ", [\"" << _lex << "\"]"
                 << std::endl;
        }
    private:
        string _lex;
};

class ast_el : public ast {
    public:
        ast_el(ast *t) : ast("ast-el", Tokens::P_RULES_EL) {
            _in = t;
        }

        virtual string get_type() override {
            /*if (_in->get_type() == "ast-el") {
                ast_el *k = (ast_el*)_in;
                return _in->get_type();
            } else*/ return _in->get_type();
        }

        ast* get_ast() {return _in;}
        ast* get_nxt() {return _nxt;}

        void addnext(ast* nxt) {
            if (_nxt != nullptr) {
                _nxt->addnext(nxt);
                return;
            }
            
            ast_el* x;
            if (nxt->get_tok() != -1) {
                if (nxt->get_tok() == Tokens::P_RULES_EL) {
                    x = (ast_el*)nxt;
                    add(nxt);
                }
                else {
                    x = new ast_el(nxt);
                    add(x);
                }
                _nxt = x;
            }
        }

        virtual void print(int INDENT = 0) override {
            cout << string(4*(INDENT), ' ')
                 << _type << std::endl;
            _in->print(INDENT+1);
            for (int i = 0; i < _chlds.size(); ++i)
                _chlds[i]->print(INDENT);
        }
    private:
        ast* _in = nullptr;
        ast_el* _nxt = nullptr;
};

class rulelist : public ast {
    public:
        rulelist() : ast("rulelist") {}
        void addnext(ast* next) {
            add(next);
        }
};

class ast_rt : public ast {
    public:
        ast_rt() : ast("start", Tokens::P_START) {}
};

class ast_or : public ast {
    public:
        ast_or(ast* rhs, ast* lhs = nullptr) : ast("or-stmt", Tokens::BAR) {
            _lhs = lhs;
            _rhs = rhs;
            add(_lhs);
            add(_rhs);
        }

        void setleft(ast* lhs) {
            if (_lhs == nullptr)
                _chlds[0] = lhs;
            else {
                ast_el *myel = (ast_el*)lhs;
                myel->addnext(_chlds[0]);
                _chlds[0] = myel;
            }
            // ((ast_el*)_chlds[0])->addnext(lhs);
            _lhs = _chlds[0];
        }

        virtual void print(int INDENT = 0) override {
            cout << string(4*(INDENT++), ' ')
                 << _type << ", " 
                 << string(tokname(_tok))
                 << std::endl;
            cout << string(4*INDENT, ' ') 
                 << "left: " << std::endl;
            _chlds[0]->print(INDENT+1);
            cout << string(4*INDENT, ' ') 
                 << "right: " << std::endl;
            _chlds[1]->print(INDENT+1);
        }

    private:
        ast* _lhs;
        ast* _rhs;
};

class ast_rule : public ast {
    public:
        ast_rule(ast* lhs, vector<ast*> rhs) : ast("rule-stmt", Tokens::P_RULE) {
            _lhs = lhs;
            _rhs = rhs;
        }

        ast* get_lhs() {return _lhs;}
        vector<ast*> get_rhs() {return _rhs;}

        virtual void print(int INDENT = 0) override {
            cout << string(4*(INDENT++), ' ')
                 << _type << ", " 
                 << string(tokname(_tok)) 
                 << std::endl;
            cout << string(4*INDENT, ' ') 
                 << "left: " << std::endl;
            _lhs->print(INDENT+1);
            cout << string(4*INDENT, ' ') 
                 << "right: " << std::endl;
            for (auto x : _rhs) {
                x->print(INDENT+1);
            }
        }

    private:
        ast* _lhs;
        vector<ast*> _rhs;
};

/**
 * @brief Contains ast's of closed semi-expressions
 * 
 */
class ast_in : public ast {
    public:
        ast_in(ast* in, Tokens op) : ast("closed-expr", op) {
            add(in);
        }

        virtual void print(int INDENT = 0) {
            cout << string(4*INDENT, ' ')
                 << _type << ", " 
                 << string(tokname(_tok))
                 << std::endl;
            for (int i = _chlds.size()-1; i >= 0; --i)
                _chlds[i]->print(INDENT+1);
        }
};

/**
 * @brief Helper function for comparing asts
 * 
 */
bool is_eq(ast* n1, ast* n2) {
    if (n1->get_type() != n2->get_type()) return false;
    if (n1->get_tok() != n2->get_tok()) {cout << "token mismatch\n";return false;}
    
    auto x = n1->getchlds();
    auto y = n2->getchlds();
    if (x.size() != y.size()) {cout << "children size mismatch\n";return false;}
    for (int i = 0; i < x.size(); ++i)
        if (!is_eq(x[i], y[i])) {cout << "child mismatch, see above\n";return false;}
    return true;
}

#endif 