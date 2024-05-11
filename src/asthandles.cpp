#include "../include/ast/asthandles.hpp"

void HandleFinder::exec() {
    generate_states();
    generate_table();
}

void HandleFinder::generate_states() {
    int i = 0;
    expand_state(_states[0]);
    while (1) {
        auto state = _states[i++];

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
            for (int g = 0; g < _states.size(); g++) {
                auto s = _states[g];
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
                        : (int)_states.size();
                HandleDictPair arg1 = std::make_pair(i-1, e);
                transitions.emplace(std::make_pair(arg1, arg2));
                if (!state_exists) _states.push_back(y);
            }
        }
        
        // break only if i passes the bounds
        // since the loop actively adds to the size of the state list
        // this is only true if a loop iteration goes without any new states being made, among other things...
        if (i == _states.size())
            break;
    }
}

void HandleFinder::generate_table() {
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
}

void HandleFinder::alt_grammar() {
    if (_states.size() <= 0) 
        return;

    for (int state = 0; state < _states.size(); state++) {
        for (auto handle : _states[state]) {
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
}

bool HandleFinder::create_first_set() {
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

bool HandleFinder::create_follow_set() {
    // compute follow sets with alt grammar
    // a simple loop; loop until break
    bool no_change = true;

    for (int i = 0; i < alt_grammar_list.size(); ++i) {
        auto item = alt_grammar_list[i]->getLeft()->getName();
        auto initsize = follow_set[item].size();
        string start = _start_state + "@0";
        if (item == start) follow_set[item].insert("$");

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

void HandleFinder::expand_state(AST_State &state) {
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
                    if (k->getLeft()->getName() == lname) {
                        state.push_back(ASTHandle(k));
                    }
                }
                alrseen.insert(lname);
            }
        }
    }
}

void HandleFinder::print_transitions() {
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

void HandleFinder::print_states() {
    cout << "printing states\n";
    for (int i = 0; i < _states.size(); i++) {
        cout << "state " << i << std::endl;
        for (auto d : _states[i]) d.print();
        cout << std::endl;
    }
    cout << std::endl;
}

void HandleFinder::print_alt_grammar() {
    cout << "printing alt grammar\n";
    for (auto a : alt_grammar_list)
        a->print();
    cout << std::endl;
}

void HandleFinder::print_first_and_follow_sets() {
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

vector<int> HandleFinder::backtrack_state(int result_state, string trigger_lit) {
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

std::vector<string> HandleFinder::get_all_terms_and_nterms() 
{
    std::vector<string> ret = {"S*", "$"};
    
    for (auto x : _lst) 
    {
        bool found = false;
        for (auto y : ret) {
            if (x->getLeft()->getName() == y) {
                found = true;
                break;
            }
        }
        if (found == false)
            ret.push_back(x->getLeft()->getName());
        for (auto k : x->getRight()->getChildren()) {
            if (k->getId() == "empty") 
                continue;
            bool found2 = false;
            Literal* l = (Literal*)k;
            string name = ((l->getId() == "tok") ? "#" : "") + l->getName();
            for (auto y : ret) {
                if (name == y) {
                    found2 = true;
                    break;
                }
            }
            if (found2 == false)
                ret.push_back(name);
        }
    }
    
    return ret;
}