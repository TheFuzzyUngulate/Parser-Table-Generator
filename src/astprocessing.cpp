#include "../include/ast/astprocessing.hpp"
#include <deque>

using std::deque;

void astproc_err(std::string err, int lineno = -1) {
    std::cerr << "processing error: "
            << err
            << ((lineno > -1) ? " at line " + std::to_string(lineno) : "") 
            << std::endl;
    exit(-1);
}

void processing_error(string err) {
    cerr << err << std::endl;
    exit(-1);
}

deque<AST*> ASTProcessor::trans6(deque<AST*> start) {
    deque<AST*> endlist = {};
    for (int i = 0; i < (int)start.size(); i++) {
        Rule* rx = (Rule*)start[i];
        bool is_found = false;
        for (int j = 0; j < (int)start.size(); j++) {
            Rule* ry = (Rule*)start[j];
            if (ry == rx && i > j) {
                is_found = true;
                break;
            }
        }
        if (!is_found)
            endlist.push_back(rx);
    }
    return endlist;
}

std::pair<bool, deque<AST*>> ASTProcessor::trans5(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        std::vector<RuleList*> newrules = {new RuleList()};
        for (auto n : nodes) {
            if (n->getId() == "orstmt") {
                not_changed = false;
                OrExpr* mynode = (OrExpr*)n;
                auto left = mynode->getLeft()->getChildren();
                auto right = mynode->getRight()->getChildren();

                auto old_size = newrules.size();
                for (int i = 0; i < old_size; ++i) {
                    auto init = newrules[i];
                    auto clone = new RuleList(*init);
                    for (auto ch : left)
                        init->addChild(ch);
                    for (auto ch : right)
                        clone->addChild(ch);
                    newrules.push_back(clone);
                }
            }
            else { 
                for (auto z : newrules)
                    z->addChild(n);
            }
        }

        for (auto z : newrules)
            endlist.push_back(new Rule(litem, z));
    }
    return make_pair(not_changed, endlist);
}

std::pair<bool, deque<AST*>> ASTProcessor::trans4(deque<AST*> start) {
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
                if (left.size() > 1 || right.size() > 1)
                    list1.push_back(mylitr);
                endlist.push_back(new Rule(litem, new RuleList(list1)));

                deque<AST*> orleft = {};
                deque<AST*> oright = {};
                orleft.insert(orleft.begin(), left.begin()+1, left.end());
                oright.insert(oright.begin(), right.begin()+1, right.end());
                if (!orleft.empty() || !oright.empty()) {
                    if (orleft.empty()) orleft.push_back(new EmptyAST());
                    if (oright.empty()) oright.push_back(new EmptyAST());
                    auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                    endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));
                }
            }
            else endlist.push_back(rule);    
        }

        else endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}

std::pair<bool, deque<AST*>> ASTProcessor::trans3(deque<AST*> start) {
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

std::pair<bool, deque<AST*>> ASTProcessor::trans2(deque<AST*> start) {
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

std::pair<bool, deque<AST*>> ASTProcessor::trans1(deque<AST*> start) {
    deque<AST*> endlist = {};
    bool not_changed = true;
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        int i = 0;
        for (; i < nodes.size(); i++) {
            // S => A {B} C becomes S => A B S', S' => B S' | C
            // S => {B} has to be S => B S' and S' => B S' | empty

            if (nodes[i]->getId() == "rep-expr") {
                not_changed = false;
                RepExpr* mynode = (RepExpr*)nodes[i];
                Literal* mylitr = new Literal(litem->getName() + '\'', Tokens::RULE);
                RuleList* inner = mynode->getExpr();
                auto innerlist = inner->getChildren();

                // S => A B S'
                if (nodes.size() > 1) {
                    deque<AST*> list1 = {};
                    list1.insert(list1.end(), nodes.begin(), nodes.begin()+i);
                    list1.insert(list1.end(), innerlist.begin(), innerlist.end());
                    list1.push_back(mylitr);
                    endlist.push_back(new Rule(litem, new RuleList(list1)));
                }

                // S' => B S' | C
                deque<AST*> orleft = {};
                deque<AST*> oright = {};
                orleft.insert(orleft.begin(), innerlist.begin(), innerlist.end());
                if (nodes.size() > 1)
                    orleft.push_back(mylitr);
                else orleft.push_back(litem);
                oright.insert(oright.begin(), nodes.begin()+i+1, nodes.end());
                if (oright.empty())
                    oright.push_back(new EmptyAST());
                
                auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                if (nodes.size() > 1)
                    endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));
                else endlist.push_back(new Rule(litem, new RuleList(orexpr)));

                // break
                break;
            }
        }
        if (i == nodes.size())
            endlist.push_back(rule);
    }
    return make_pair(not_changed, endlist);
}

/**
 * @brief Check whether a start non-terminal is present in the ruleset
 * 
 * @param start Ruleset being tested
 * @return true if present, and
 * @return false otherwise
 */
bool ASTProcessor::semcheck1(deque<AST*> start) {
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        if (litem->getName() == "START")
            return true;
    } return false;
}

/**
 * @brief Check whether all non-terminals are defined somewhere in the ruleset
 * 
 * @param start Ruleset being tested
 * @return true, if all non-terminals are indeed defined in the ruleset, and
 * @return false otherwise.
 */
bool ASTProcessor::semcheck2(deque<AST*> start) {
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto rchilds = rule->getRight()->getChildren();
        for (int i = 0; i < rchilds.size(); ++i) {
            auto symb = rchilds[i];
            if (symb->getId() == "lit") {
                Literal* my_lt = (Literal*)symb;
                auto find_res = std::find(alphabet.begin(), alphabet.end(), my_lt->getName());
                if (find_res == alphabet.end()) {
                    astproc_err("undefined non-terminal " + my_lt->getName() + " present", i);
                }
            }
        }
    } return true;
}

void ASTProcessor::setsymbs(deque<AST*> lst) {
    for (auto r : lst) {
        Rule* rule = (Rule*)r;
        
        auto litem = rule->getLeft();
        alphabet.insert(litem->getName());

        auto rchilds = rule->getRight()->getChildren();
        for (auto symb : rchilds) {
            auto t = symb->getId();
            if (t == "tok") {
                Literal* my_tk = (Literal*)symb;
                alphabet.insert("#" + my_tk->getName());
            }
            else
            if (t == "empty") {
                if (nontermlist.find("empty") != nontermlist.end())
                    alphabet.insert("empty");
            }
        }
    }
}

deque<AST*> ASTProcessor::process_ast_ll1() {
    auto children = _start->getChildren();
    deque<AST*> res_holder = _start->getChildren();
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
    }
    auto q = trans6(res_holder);
    setsymbs(q);
    return q;
}

deque<AST*> ASTProcessor::process_ast_lalr1() {
    auto children = _start->getChildren();
    deque<AST*> res_holder = _start->getChildren();
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
            
            res1 = trans5(res_holder);
            no_change = no_change && res1.first;
            res_holder = res1.second;
        }
        if (no_change)
            break;
    }

    auto q = trans6(res_holder);
    setsymbs(q);
    if (!semcheck1(q))
        astproc_err("no start state found");
    if (!semcheck2(q))
        astproc_err("undefined rule used");
    return q;
}

std::set<std::string> ASTProcessor::get_alphabet() {
    return alphabet;
}