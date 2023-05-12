#ifndef ASTPROCESSING_HPP
#define ASTPROCESSING_HPP
#pragma once

#include "ast.hpp"
#include "parser.hpp"

/**
 * Note that all lists are in reverse order; all except the righthandside of an
 * or statement, which is in the correct order. I COULD rectify that, but it's a
 * waste of time, honestly. If it gets too unbearable, I'll do it. 
 **/
//sdsds

void processing_error(string err) {
    cerr << err << std::endl;
    exit(-1);
}

std::pair<bool, deque<AST*>> trans5(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        if (nodes[0]->getId() == "orstmt") {
            not_changed = false;
            OrExpr* mynode = (OrExpr*)nodes[0];
            auto left = mynode->getLeft();
            auto right = mynode->getRight();
            endlist.push_back(new Rule(litem, left));
            endlist.push_back(new Rule(litem, right));
        }
        else endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}


std::pair<bool, deque<AST*>> trans4(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        if (nodes[0]->getId() == "orstmt") {
            // S => A B | A C yields S => A S' and S' => B C | empty
            OrExpr* mynode = (OrExpr*)nodes[0];
            Literal* mylitr = new Literal(litem->getName() + "\'", Tokens::RULE);
            auto left = mynode->getLeft()->getChildren();
            auto right = mynode->getRight()->getChildren();

            if (left[0]->getId() == "lit" 
            && right[0]->getId() == "lit"
            && ((Literal*)left[0])->getName() == ((Literal*)right[0])->getName()) {
                Literal* mynode = (Literal*)left[0];
                if (litem->getName() == mynode->getName())
                    processing_error("grammar not in LL(1)");
                not_changed = false;
                deque<AST*> list1 = {};
                list1.push_back(mynode);
                list1.push_back(mylitr);
                endlist.push_back(new Rule(litem, new RuleList(list1)));

                deque<AST*> orleft = {};
                deque<AST*> oright = {};
                orleft.insert(orleft.begin(), left.begin()+1, left.end());
                oright.insert(oright.begin(), right.begin()+1, right.end());
                auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));
            }
            else endlist.push_back(rule);    
        }

        else endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}


std::pair<bool, deque<AST*>> trans3(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        if (nodes[0]->getId() == "orstmt") {
            // S => S B | C yields S => C S' and S' => B S' | empty
            OrExpr* mynode = (OrExpr*)nodes[0];
            Literal* mylitr = new Literal(litem->getName() + "\'", Tokens::RULE);
            auto left = mynode->getLeft()->getChildren();
            auto right = mynode->getRight()->getChildren();

            bool in_left = left[0]->getId() == "lit" && litem->getName() == ((Literal*)left[0])->getName();
            bool in_right = right[0]->getId() == "lit" && litem->getName() == ((Literal*)right[0])->getName();

            if (!in_left && !in_right)
                endlist.push_back(rule);
            else {
                if (!in_left || !in_right) {
                    not_changed = false;
                    auto r1 = (in_left) ? left : right;
                    auto r2 = (in_right) ? left : right;
                    
                    deque<AST*> list1 = {};
                    list1.insert(list1.begin(), r2.begin(), r2.end());
                    list1.push_back(mylitr);
                    endlist.push_back(new Rule(litem, new RuleList(list1)));

                    deque<AST*> orleft = {};
                    deque<AST*> oright = {};
                    orleft.insert(orleft.begin(), r1.begin()+1, r1.end());
                    orleft.push_back(mylitr);
                    oright.push_back(new EmptyAST());
                    auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                    endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));
                }
                else processing_error("grammar not in LL(1)");
            }
        }
        else {
            if (nodes[0]->getId() == "lit") {
                Literal* myn = (Literal*)nodes[0];
                if (litem->getName() == myn->getName())
                    processing_error("grammar not in LL(1)");
            } endlist.push_back(rule);
        }
    }
    return make_pair(not_changed, endlist);
}


std::pair<bool, deque<AST*>> trans2(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        int i = 0;
        for (; i < nodes.size(); i++) {
            // S => A [B] C becomes S => A S' and S' => B C | C
            // of course, if the preset is empty, then it's just S => B C | C for S => [B] C

            if (nodes[i]->getId() == "opt-expr") {
                not_changed = false;
                OptExpr* mynode = (OptExpr*)nodes[i];
                Literal* mylitr = new Literal(litem->getName() + '\'', Tokens::RULE);
                RuleList* inner = mynode->getExpr();
                auto innerlist = inner->getChildren();

                deque<AST*> list1 = {};
                list1.insert(list1.end(), nodes.begin(), nodes.begin()+i);
                if (list1.size() > 0) {
                    list1.push_back(mylitr);
                    endlist.push_back(new Rule(litem, new RuleList(list1)));
                }

                deque<AST*> orleft = {};
                deque<AST*> oright = {};
                orleft.insert(orleft.begin(), innerlist.begin(), innerlist.end());
                orleft.insert(orleft.end(), nodes.begin()+i+1, nodes.end());
                oright.insert(oright.begin(), nodes.begin()+i+1, nodes.end());
                if (oright.empty())
                    oright.push_back(new EmptyAST());
                auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                if (list1.size() > 0)
                    endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));
                else
                    endlist.push_back(new Rule(litem, new RuleList(orexpr)));
                break;
            }
        }
        if (i == nodes.size())
            endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}


std::pair<bool, deque<AST*>> trans1(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        int i = 0;
        for (; i < nodes.size(); i++) {
            // if something is {} or [], get it
            // then, replace accordingly. we start with "{}"
            // S => A {B} C becomes S => A B S', S' => B S' | C
            // S => {B} has to be S => B S' and S' => B S' | empty

            if (nodes[i]->getId() == "rep-expr") {
                not_changed = false;
                RepExpr* mynode = (RepExpr*)nodes[i];
                Literal* mylitr = new Literal(litem->getName() + '\'', Tokens::RULE);
                RuleList* inner = mynode->getExpr();
                auto innerlist = inner->getChildren();

                // S => A B S'
                deque<AST*> list1 = {};
                list1.insert(list1.end(), nodes.begin(), nodes.begin()+i);
                list1.insert(list1.end(), innerlist.begin(), innerlist.end());
                list1.push_back(mylitr);
                endlist.push_back(new Rule(litem, new RuleList(list1)));

                // S' => B S' | C
                deque<AST*> orleft = {};
                deque<AST*> oright = {};
                orleft.insert(orleft.begin(), innerlist.begin(), innerlist.end());
                orleft.push_back(mylitr);
                oright.insert(oright.begin(), nodes.begin()+i+1, nodes.end());
                if (oright.empty())
                    oright.push_back(new EmptyAST());
                
                auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));

                // break
                break;
            }
        }
        if (i == nodes.size())
            endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}

deque<AST*> process_ast(StartAST* start) {
    auto children = start->getChildren();
    deque<AST*> res_holder = start->getChildren();
    while (1) {
        bool no_change = true;
        for (auto child : children) {
            Rule* myrule = (Rule*)child;
            std::pair<bool, deque<AST*>> res1;
            
            res1 = trans1(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;

            res1 = trans2(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;

            res1 = trans3(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;

            res1 = trans4(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;

            res1 = trans5(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;
        }
        if (no_change)
            break;
        cout << "continuing...\n";
    }
    return res_holder;
}

#endif