#ifndef PROCREGEX_HPP
#define PROCREGEX_HPP
#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <exception>

class QueueItem {
    public:
        QueueItem *parent;
        QueueItem *nxt;
        QueueItem *prev;
        bool isNP() {return _is_np;}
        std::string getName() {return _name;}
        QueueItem(std::string name) : _name(name), parent(nullptr), nxt(nullptr), _is_np(false) {}
        virtual void print(int INDENT = 0) {
            throw std::runtime_error("can't print, can't print");
        }
    protected:
        std::string _name;
        bool _is_np;
};


class NonPrimitive : public QueueItem {
    public:
        NonPrimitive(std::string name) : QueueItem(name) {}
        virtual void addChild(QueueItem* child) = 0;
        virtual QueueItem* removeChild() = 0;
        virtual QueueItem *getChild() = 0;
        virtual void graft() = 0;
};

class OptNonPrimitive : public NonPrimitive {
    public:
        OptNonPrimitive(std::string name) : NonPrimitive(name) {
            _is_np = true;
            addLevel();
        }
        void addLevel(QueueItem* head) {
            levels.push_back(head);
        }
        int levelCount() {return levels.size();}
        void addLevel() {addLevel(nullptr);}
        QueueItem* at(int index) {
            return levels[index];
        }
        virtual QueueItem *getChild() {
            return levels[levels.size()-1];
        }
        virtual void graft() {
            throw std::runtime_error("can't graft from an optnoprimitive");
        }
        void addChild(QueueItem* child) override;
        QueueItem* removeChild() override;
        ssize_t size() {return levels.size();}
        virtual void print(int INDENT=0) override;

    private:
        std::vector<QueueItem*> levels;
};

class NormNonPrimitive : public NonPrimitive {
    public:
        NormNonPrimitive(std::string name) : NonPrimitive(name), _child(nullptr) {
            _is_np = true;
        }
        virtual QueueItem *getChild() override {return _child;}
        virtual void addChild(QueueItem* child);
        virtual void graft() override;
        virtual QueueItem* removeChild();
        virtual void print(int INDENT=0) override;
    private:
        QueueItem *_child;
};

class Primitive : public QueueItem {
    public:
        Primitive(std::string id) : QueueItem(id) {}
        virtual void print(int INDENT=0) override;
};

class RegExChecker {
    public:
        RegExChecker(std::string reg, std::string in) : _index(0), _expr(load()), _instr(in), _str(reg) {}
        bool verify(QueueItem *expr);
        bool verify() {return verify(_expr->getChild());}

    private:
        int _index;
        std::string _str;
        std::string _instr;
        NonPrimitive *_expr;
        NonPrimitive *load();
        bool inner(QueueItem *expr);
};


/* int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Incorrecto!!\n";
        return EXIT_FAILURE;
    }

    auto inputstr = std::string(argv[1]);
    auto inputstr2 = std::string(argv[2]);
    auto regex = RegExChecker(inputstr, inputstr2);
    std::cout << (regex.verify() ? "true" : "false");

    return EXIT_SUCCESS;
} */

#endif