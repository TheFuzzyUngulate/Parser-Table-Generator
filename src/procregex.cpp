#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <exception>
#include "../include/regex/procregex.hpp"

void OptNonPrimitive::addChild(QueueItem* child) {
    child->parent = this;
    if (levels.size() > 0) {
        auto old = levels[levels.size()-1];
        if (old != nullptr) {
            while (old->nxt != nullptr) old = old->nxt;
            old->nxt = child;
        } else levels[levels.size()-1] = child;
    } else levels.push_back(child);
}

QueueItem* OptNonPrimitive::removeChild() {
    if (levels.size() > 0) {
        auto old = levels[levels.size()-1];
        if (old->nxt == nullptr) {
            levels[levels.size()-1] = nullptr;
            old->parent = nullptr;
            return old;
        }
        else {
            while (old->nxt->nxt != nullptr) old = old->nxt;
            auto ret = old->nxt;
            old->nxt = nullptr;
            ret->parent = nullptr;
            return ret;
        }
    } else throw std::runtime_error("no child available.");
}

void OptNonPrimitive::print(int INDENT=0) {
    std::cout << std::string(INDENT*4, ' ') << _name << ":\n";
    for (int i = 0; i < levels.size(); ++i) {
        std::cout << std::string((INDENT+1)*4, ' ') << "level " << std::to_string(i) << ":\n";
        auto ptr = levels[i];
        while (ptr != nullptr) {
            ptr->print(INDENT + 2);
            ptr = ptr->nxt;
        }
    }
}

// Non-primitive Normal Nodes

void NormNonPrimitive::addChild(QueueItem* child) {
    child->parent = this;
    if (_child != nullptr) {
        auto old = _child;
        while (old->nxt != nullptr) 
            old = old->nxt;
        old->nxt = child;
    } else _child = child;
}

void NormNonPrimitive::graft() {
    auto q = new OptNonPrimitive("alt");
    q->addChild(_child);
    q->parent = this;
    q->addLevel();
    _child = q;
}

QueueItem* NormNonPrimitive::removeChild() {
    if (_child != nullptr) {
        auto old = _child;
        if (old->nxt == nullptr) {
            _child = nullptr;
            old->parent = nullptr;
            return old;
        }
        else {
            while (old->nxt->nxt != nullptr)
                old = old->nxt;
            auto ret = old->nxt;
            old->nxt = nullptr;
            ret->parent = nullptr;
            return ret;
        }
    } else throw std::runtime_error("no child available.");
}

void NormNonPrimitive::print(int INDENT=0) {
    std::cout << std::string(INDENT*4, ' ') << _name << ":\n";
    auto ptr = _child;
    while (ptr != nullptr) {
        ptr->print(INDENT + 1);
        ptr = ptr->nxt;
    }
}

// Primitive Nodes

void Primitive::print(int INDENT=0) {
    std::cout << std::string(INDENT*4, ' ') << "prim: " << _name << std::endl;
}

// RegEx Checker

bool RegExChecker::verify(QueueItem *expr) {
    int old = _index;
    if (!inner(expr)) {
        _index = old;
        return false;
    } else return true;
}

NonPrimitive *RegExChecker::load() {
    NonPrimitive *head = new NormNonPrimitive("root");
    NonPrimitive *top = head;
    int i = 0;
    while (i < _str.size()) {
        char x = _str[i++];
        switch (x) {
            case '{': {
                auto k = new NormNonPrimitive("rep");
                top->addChild(k);
                top = k;
                break;
            }
            case '(': {
                auto k = new NormNonPrimitive("close");
                top->addChild(k);
                top = k;
                break;
            }
            case '[': {
                auto k = new NormNonPrimitive("select");
                top->addChild(k);
                top = k;
                break;
            }
            case '?': {
                auto r = top->removeChild();
                auto k = new NormNonPrimitive("opt");
                k->addChild(r);
                top->addChild(k);
                break;
            }
            case '|': {
                if (top->getName() != "alt") {
                    top->graft();
                    top = (NonPrimitive*)top->getChild();
                } else ((OptNonPrimitive*)top)->addLevel();
                break;
            }
            case '\\': {
                char x = _str[i++];
                std::string str = "\\";
                str += x;
                top->addChild(new Primitive(str));
                break;
            }
            case '}':
            case ']':
            case ')':
                if (top->getName() == "alt")
                    top = (NonPrimitive*)top->parent->parent;
                else top = (NonPrimitive*)top->parent;
                break;
            default: {
                std::string str;
                str += x;

                if (i < _str.size())
                {
                    char y = _str[i++];
                    if (y == '-' && i < _str.size())
                    {
                        y = _str[i++];
                        if ((isalpha(x) == isalpha(y)) && (isdigit(x) == isdigit(y)))
                        {
                            str += '-';
                            str += y;
                            top->addChild(new Primitive(str));
                            break;
                        } else i -= 2;
                    } else --i;
                }

                top->addChild(new Primitive(str));
                break;
            }
        }
    }
    head->print();
    return head;
}

bool RegExChecker::inner(QueueItem* expr) {
    if (expr == nullptr)
        return true;
    if (expr->isNP())
    {
        if (expr->getName() == "alt")
        {
            auto alt = (OptNonPrimitive*)expr;
            auto fst = alt->at(0);
            switch(alt->levelCount())
            {
                case 1:
                    return verify(fst);
                case 2: {
                    auto snd = alt->at(1);
                    if (verify(fst))
                        return true;
                    else return verify(snd);
                }
                default: {
                    auto snd = new OptNonPrimitive("alt");
                    for (int i = 1; i < alt->levelCount(); ++i) {
                        if (alt->at(i) != nullptr) {
                            snd->addChild(alt->at(i));
                            snd->addLevel();
                        }
                    }
                    if (verify(fst)) {
                        return true;
                    } else return verify(snd);
                }
            }
        }
        else
        {
            auto fst = (NormNonPrimitive*)expr;
            auto snd = fst->nxt;

            if (fst->getName() == "rep") {
                auto ofst = fst->getChild();
                do {
                    if (verify(snd))
                        return true;
                } while (verify(ofst));
                return false;
            }

            if (fst->getName() == "opt") {
                auto ofst = fst->getChild();
                verify(ofst);
                return verify(snd);
            }

            if (fst->getName() == "close") {
                auto body = fst->getChild();
                if (verify(body))
                    return verify(snd);
                else return false;
            }

            if (fst->getName() == "select") {
                auto ofst = fst->getChild();
                if (ofst->nxt != nullptr)
                {
                    auto osnd = new OptNonPrimitive("select");
                    osnd->nxt = snd;
                    osnd->addChild(ofst->nxt);
                    ofst->nxt = nullptr;
                    bool tr = verify(ofst);
                    ofst->nxt = osnd->getChild();

                    if (tr) {
                        return verify(snd);
                    } else return verify(osnd);
                } 
                else {
                    if (verify(ofst))
                        return verify(snd);
                    else return false;
                }
            }
        }
    }
    else
    {
        auto t_expr = (Primitive*)expr;
        auto exp_name = t_expr->getName();
        auto act_name = std::string();
        act_name.push_back(_instr[_index]);

        if (exp_name == "$")
            return (_index >= _instr.size());

        bool gotten;
        if (exp_name.size() == 2 && exp_name[0] == '\\')
        {
            auto a = act_name[0];
            auto ch = exp_name[1];
            switch (ch)
            {
                case 'd': 
                    gotten = isdigit(a);
                    break;
                case 'a':
                    gotten = isalnum(a);
                    break;
                case 'w': 
                    gotten = isalpha(a) && islower(a);
                    break;
                case 'W':
                    gotten = isalpha(a) && isupper(a);
                    break;
                default: gotten = a == ch;
            }
        }
        else
        if (exp_name.size() == 3
            && exp_name[1] == '-'
            && (isdigit(exp_name[0]) == isdigit(exp_name[2]))
            && (isalpha(exp_name[0]) == isalpha(exp_name[2]))) {
                auto a = act_name[0];
                gotten = (exp_name[0] <= a) && (a <= exp_name[2]);
        }
        else gotten = act_name == exp_name;

        if (gotten) {
            ++_index;
            return verify(t_expr->nxt);
        } else return false;
    }

    return false;
}