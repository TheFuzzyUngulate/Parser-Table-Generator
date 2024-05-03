#include "../include/ast/astprocessing.hpp"
#include <deque>
#include <sstream>

using std::deque,
      std::string;
typedef std::map<std::string, std::set<std::string>> FollowSetMap;

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
    for (int i = 0; i < start.size(); i++) {
        Rule* rule = (Rule*)start[i];
        auto litem = rule->getLeft();
        auto rlist = rule->getRight();
        deque<AST*> nodes = rlist->getChildren();

        /*if (nodes[0]->getId() == "orstmt") {
            // S => (A | B) C yields S => A C and S => B C
            // though, this only does it one at a time, because reasons...
            // so a break statement is necessary
            not_changed    = false;
            OrExpr* mynode = (OrExpr*)nodes[0];
            auto left      = mynode->getLeft()->getChildren();
            auto right     = mynode->getRight()->getChildren();

            deque<AST*> newA = {};
            deque<AST*> newB = {};

            newA.insert(newA.begin(), left.begin(), left.end());
            newB.insert(newB.begin(), right.begin(), right.end());

            for (i=i+1; i < start.size(); ++i) 
            {
                AST* node2 = start[i];
                newA.push_back(node2);
                newB.push_back(node2);
            }
            
            endlist.push_back(new Rule(litem, new RuleList(newA)));
            endlist.push_back(new Rule(litem, new RuleList(newB)));
        }

        else endlist.push_back(rule);*/

        int k;
        std::vector<RuleList*> newrules = {new RuleList()};

        for (k = 0; k < nodes.size(); ++k)
        {
            auto n = nodes[k];

            if (n->getId() == "orstmt") {
                not_changed    = false;
                OrExpr* mynode = (OrExpr*)n;
                auto left      = mynode->getLeft()->getChildren();
                auto right     = mynode->getRight()->getChildren();

                deque<AST*> newA = {};
                deque<AST*> newB = {};
                auto earliers    = newrules[0]->getChildren();

                newA.insert(newA.end(), nodes.begin(), nodes.begin()+k);
                newB.insert(newB.end(), nodes.begin(), nodes.begin()+k);
                
                newA.insert(newA.end(), left.begin(), left.end());
                newB.insert(newB.end(), right.begin(), right.end());

                newA.insert(newA.end(), nodes.begin()+k+1, nodes.end());
                newB.insert(newB.end(), nodes.begin()+k+1, nodes.end());

                endlist.push_back(new Rule(litem, new RuleList(newA)));
                endlist.push_back(new Rule(litem, new RuleList(newB)));

                break;
            }
        }

        if (k == nodes.size())
            endlist.push_back(rule);

        /**std::vector<RuleList*> newrules = {new RuleList()};
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
                    for (int k = 0; k < left.size(); ++k)
                        init->addChild(left[k]);
                    for (int k = 0; k < right.size(); ++k)
                        clone->addChild(right[k]);
                    newrules.push_back(clone);
                }
            }
            else { 
                for (auto z : newrules)
                    z->addChild(n);
            }
        }

        for (auto z : newrules)
            endlist.push_back(new Rule(litem, z));**/
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
            // S => A {B} C becomes S => A S' C, S' => B S' | empty
            // S => {B} has to be S => B S | empty

            if (nodes[i]->getId() == "rep-expr") {
                not_changed = false;
                RepExpr* mynode = (RepExpr*)nodes[i];
                Literal* mylitr = new Literal(litem->getName() + '\'', Tokens::RULE);
                RuleList* inner = mynode->getExpr();
                auto innerlist = inner->getChildren();

                /*// S => A B S'
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
                if (oright.empty()) oright.push_back(new EmptyAST());
                auto orexpr = new OrExpr(new RuleList(orleft), new RuleList(oright));
                endlist.push_back(new Rule(mylitr, new RuleList(orexpr)));*/

                // S => {B}
                if (nodes.size() == 1) {
                    // S => B S
                    deque<AST*> list1  = {};
                    list1.insert(list1.begin(), innerlist.begin(), innerlist.end());
                    list1.push_back(litem);
                    endlist.push_back(new Rule(litem, new RuleList(list1)));
                    
                    // S => empty
                    endlist.push_back(new Rule(litem, new RuleList(new EmptyAST())));
                }

                // S => A {B} C
                else {
                    // S => A S' C
                    deque<AST*> list1 = {};
                    list1.insert(list1.end(), nodes.begin(), nodes.begin()+i);
                    list1.push_back(mylitr);
                    list1.insert(list1.end(), nodes.begin()+i+1, nodes.end());
                    endlist.push_back(new Rule(litem, new RuleList(list1)));

                    // S' => B S'
                    deque<AST*> list2 = {};
                    list2.insert(list2.begin(), innerlist.begin(), innerlist.end());
                    list2.push_back(mylitr);
                    endlist.push_back(new Rule(mylitr, new RuleList(list2)));

                    // S' => empty
                    endlist.push_back(new Rule(mylitr, new RuleList(new EmptyAST())));
                }
                
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
bool ASTProcessor::semcheck1(deque<AST*> start, string start_state) {
    if (start_state == "")
        processing_error("no start state specified");
    for (auto x : start) {
        Rule* rule = (Rule*)x;
        auto litem = rule->getLeft();
        if (litem->getName() == start_state)
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

bool first_sets(deque<AST*> asts, FollowSetMap &frstset, FollowSetMap &folwset)
{
    int i          = 0;
    int k          = 0;
    bool no_change = true;

    for (i = 0; i < asts.size(); ++i)
    {
        auto left           = ((Rule*)asts[i])->getLeft()->getName();
        auto initsize       = frstset[left].size();
        auto endsize        = 0;
        auto rhs_items      = ((Rule*)asts[i])->getRight()->getChildren();
        auto rhs_first      = rhs_items.at(0);
        auto rhs_first_name = std::string();

        if (rhs_first->getId() == "lit") {
            
            // add epsilon cover of first set
            for (k = 0; k < rhs_items.size(); ++k) {
                rhs_first_name = ((Literal*)rhs_items.at(k))->getName();
                if (frstset.find(rhs_first_name) != frstset.end()) {
                    for (auto fsvals : frstset[rhs_first_name]) {
                        if (fsvals != "?") 
                            frstset[left].insert(fsvals);
                    }
                    if (frstset[rhs_first_name].find("?") == frstset[rhs_first_name].end()) 
                        break;
                } else break;
            }

            // if all terms could be epsilon, add '?' to first set (epsilon)
            if (k == rhs_items.size())
                frstset[left].insert("?");
        }
        else if (rhs_first->getId() == "tok") {
            rhs_first_name = ((Literal*)rhs_first)->getName();
            frstset[left].insert(rhs_first_name);
        }
        else {
            if (rhs_first->getId() == "empty") {
                frstset[left].insert("?");
            }
        }

        endsize   = frstset[left].size();
        no_change = no_change && (initsize == endsize);
    }

    return no_change;
}

bool follow_sets(deque<AST*> asts, FollowSetMap &frstset, FollowSetMap &folwset, string start)
{
    int i             = 0;
    string left       = "";
    int endsize       = 0;
    int initsize      = 0;
    int k             = 0;
    string curr       = "";
    AST* last         = NULL;
    string next       = "";
    bool no_change    = true;
    deque<AST*> right = {};

    for (i = 0; i < asts.size(); ++i)
    {
        left  = ((Rule*)asts[i])->getLeft()->getName();
        right = ((Rule*)asts[i])->getRight()->getChildren();
        
        if (left == start) {
            if (folwset[left].find("$") == folwset[left].end()) {
                no_change = false;
                folwset[left].insert("$");
            }
        }
        
        for (k = 0; k < right.size() - 1; ++k) 
        {
            if (right[k]->getId() == "lit") {

                curr     = ((Literal*)right[k])->getName();
                next     = ((Literal*)right[k+1])->getName();
                initsize = folwset[curr].size();

                if (right[k+1]->getId() == "lit") {
                    // add all non-eps elements in first set of next
                    for (auto folw : frstset[next]) {
                        if (folw != "?") folwset[curr].insert(folw);
                    }

                    // if next element is last, and it might be eps, then...
                    if (k+1 == right.size() - 1) {
                        if (frstset[next].find("?") != frstset[next].end()) {
                            for (auto folw : folwset[left])
                                folwset[curr].insert(folw);
                        }
                    }
                }

                // tok is easier, just add it
                if (right[k+1]->getId() == "tok")
                    folwset[curr].insert(next);

                endsize   = folwset[curr].size();
                no_change = no_change && (initsize == endsize);
            }
        }

        /* handle last item */
        last = right[right.size() - 1];

        if (last->getId() == "lit") {

            curr     = ((Literal*)last)->getName();
            initsize = folwset[curr].size();
            
            for (auto folw : folwset[left])
                folwset[curr].insert(folw);

            endsize   = folwset[curr].size();
            no_change = no_change && (initsize == endsize);
        }
    }

    return no_change;
}

deque<AST*> ASTProcessor::process_ast_ll1() {
    auto children = _start->getChildren();
    deque<AST*> childtmp = _start->getChildren();
    while (1) {
        bool no_change = true;
        for (auto child : children) {
            Rule* myrule = (Rule*)child;
            std::pair<bool, deque<AST*>> res1;
            
            res1 = trans1(childtmp);
            no_change = no_change && res1.first;
            childtmp = res1.second;

            res1 = trans2(childtmp);
            no_change = no_change && res1.first;
            childtmp = res1.second;

            res1 = trans3(childtmp);
            no_change = no_change && res1.first;
            childtmp = res1.second;

            res1 = trans4(childtmp);
            no_change = no_change && res1.first;
            childtmp = res1.second;

            res1 = trans5(childtmp);
            no_change = no_change && res1.first;
            childtmp = res1.second;
        }
        if (no_change)
            break;
    }
    auto q = trans6(childtmp);
    setsymbs(q);
    return q;
}

deque<AST*> ASTProcessor::process_ast_lalr1(string start_state) {
    int index             = 0;
    int ruleind           = 0;
    bool no_change        = true;
    deque<AST*> children  = _start->getChildren();
    deque<AST*> childtmp = _start->getChildren();
    
    std::pair<bool, deque<AST*>> result;

    if (_showProc) {
        printf("At start of processing:\n");
        for (auto child : children)
            child->print();
        printf("\n");
    }

    while (true)
    {
        no_change = true;

        for (index = 0; index < children.size(); ++index)
        {
            Rule* rule = (Rule*)children[index];
            auto litem = rule->getLeft();
            auto rlist = rule->getRight();
            deque<AST*> nodes = rlist->getChildren();

            for (ruleind = 0; ruleind < nodes.size(); ++ruleind)
            {
                string id = nodes[ruleind]->getId();

                if (id == "orstmt")        result = trans5({rule});
                else if (id == "rep-expr") result = trans1({rule});
                else if (id == "opt-expr") result = trans2({rule});
                else continue;

                if (!result.first) {
                    if (_showProc) {
                        cout << "old state:\n";
                        rule->print(1);
                    }

                    childtmp.clear();
                    childtmp.insert(childtmp.end(), children.begin(), children.begin()+index);
                    childtmp.insert(childtmp.end(), result.second.begin(), result.second.end());
                    childtmp.insert(childtmp.end(), children.begin()+index+1, children.end());
                    children = childtmp;

                    if (_showProc) {
                        cout << "new states after "
                            << (id == "orstmt" ? "alt resolution" :
                                (id == "rep-expr" ? "repetition resolution" :
                                "optional resolution"))
                            << " \n";
                        for (auto res : result.second)
                            res->print(1);
                        cout << "\n";
                    }

                    break;
                }
            }

            if (ruleind < nodes.size()) break;
        }

        if (index == children.size()) 
            break;
    }

    childtmp = trans6(childtmp);
    setsymbs(childtmp);

    if (!semcheck1(childtmp, start_state))
        astproc_err("no start state found");

    if (!semcheck2(childtmp))
        astproc_err("undefined rule used");

    FollowSetMap fst;
    FollowSetMap flw;

    for (auto ast : childtmp) {
        fst[((Rule*)ast)->getLeft()->getName()] = {};
        flw[((Rule*)ast)->getLeft()->getName()] = {};
    }
    
    while (!first_sets(childtmp, fst, flw) 
    || !follow_sets(childtmp, fst, flw, start_state));

    for (auto ast : childtmp) {
        auto rule   = (Rule*)ast;
        auto flwset = flw[rule->getLeft()->getName()];
        for (auto item : flwset)
            rule->addFollow(item);
    }
    
    return childtmp;
}

std::set<std::string> ASTProcessor::get_alphabet() {
    return alphabet;
}