#include "../include/gen/codegen.hpp"

void
CodeGenerator::ast_create_groups()
{
    int i, k;
    int initcount;

    /**
     * Create grouping for rules.
     * Rules R, S can only be in the same group if:
     * 
     *    1. R.LHS is equivalent to S.LHS,
     * 
     *    2. R.RHS is of the same length as S.RHS, and
     * 
     *    3. For each index i in range 1..length(R.RHS),
     *       at least one of the following must be the case:
     *        a. R.RHS[i] and S.RHS[i] are both tokens, or
     *        b. R.RHS[i] and S.RHS[i] are identical literals.
     */

    for (i = 0; i < _rules.size(); ++i)
    {
        bool flag = false;
        auto rulf = _rules[i]->getLeft()->getName();
        auto ruch = _rules[i]->getRight()->getChildren();

        if (ruch[0]->getId() == "empty") continue;

        for (auto &group : _astgroups)
        {
            auto ex   = group[0].second;
            auto exlf = ex->getLeft()->getName();

            if (exlf == rulf)
            {
                auto exch = ex->getRight()->getChildren();

                if (exch.size() == ruch.size())
                {
                    for (k = 0; k < exch.size(); ++k)
                    {
                        if (exch[k]->getId() != ruch[k]->getId()) break;

                        /*if (exch[k]->getId() == "lit") {
                            auto exi = ((Literal*)exch[k]);
                            auto rui = ((Literal*)ruch[k]);
                            if (rui->getName() != exi->getName()) break;
                        }*/
                    }

                    if (k == exch.size()) {
                        group.push_back({i, _rules[i]});
                        flag = true;
                        break;
                    }
                }
            }
        }

        if (!flag) {
            _astgroups.push_back({std::make_pair(i, _rules[i])});
        }
    }
}

void
CodeGenerator::ast_create_groupnames()
{
    int i;
    std::deque<std::pair<int, std::string>> seencount;

    for (auto item : _astgroups)
    {
        auto a   = item[0].second;
        auto lhs = a->getLeft()->getName();

        for (i = 0; i < seencount.size(); ++i) {
            if (lhs == seencount[i].second) {
                _astgroupnames.push_back(
                    _elementtoks[lhs] + std::to_string(seencount[i].first++)
                );
                break;
            }
        }

        if (i == seencount.size()) {
            _astgroupnames.push_back(_elementtoks[lhs]);
            seencount.push_back({1, lhs});
        }
    }
}

void CodeGenerator::generate()
{
    int i,j,k;
    int state;
    std::string line;
    std::string name;
    std::string regex;
    regopts subst;
    std::string delim;
    std::ifstream templ;
    std::ofstream ofile;

    delim = "/* stop */";

    ofile = std::ofstream();
    templ = std::ifstream();

    state = 0;
    templ.open("res/ptgparse.h.template");
    ofile.open("out/" + _fname + ".h");
    while (std::getline(templ, line))
    {
        /* if we aren't signalled to insert, leave it */
        if (line != delim) {
            ofile << line << "\n";
            continue;
        }

        /* otherwise... depends on state */
        switch (state)
        {
            case 0:
            {
                /* add scanner token enum */
                i = 0;
                for (auto el : _elements) {
                    ofile << "\t" 
                          << _elementtoks[el]
                          << ((i != _elements.size()-1) ? "," : "")
                          << "\n";
                    ++i;
                }

                /* break state */
                break;
            }

            case 1:
            {
                /* add string representation of tokens */
                ofile << "\tswitch(tok) {\n";
                for (auto el : _elements) {
                    ofile << "\t\tcase " 
                          << _elementtoks[el]
                          << ": return \""
                          << el << "\";\n";
                } ofile << "\t\tdefault: return \"?\";\n\t}\n";

                /* break state */
                break;
            }

            case 2:
            {
                /* should first create groups and groupnames */
                ast_create_groups();
                ast_create_groupnames();

                /* add an identifier for each groupname */
                for (i = 0; i < _astgroupnames.size(); ++i) {
                    ofile << "\t\tPTGAST_" << _astgroupnames[i]
                          << ((i != _astgroupnames.size() - 1) ? "," : "")
                          << "\n";
                }

                /* break state */
                break;
            }

            case 3:
            {
                /* loop through all groupings */
                for (i = 0; i < _astgroups.size(); ++i) {
                    auto groupitm = _astgroups[i][0].second;
                    auto rhsitems = groupitm->getRight()->getChildren();

                    /* if singleton, use either ptgast or string datatype */
                    if (rhsitems.size() == 1) {
                        auto single = rhsitems[0];
                        if (single->getId() != "empty") {
                            if (single->getId() == "lit") {
                                auto singlit = (Literal*)single;
                                ofile << "\t\tstruct ptgast* " << _astgroupnames[i] << ";\n";
                            }
                            else 
                            if (single->getId() == "tok") {
                                auto singlit = (Literal*)single;
                                ofile << "\t\tchar* " << _astgroupnames[i] << ";\n";
                            }
                        }
                    }

                    /* otherwise, you have to create a compound struct */
                    else {
                        ofile << "\t\tstruct {\n";

                        int tokcount  = 0;
                        int nodecount = 0;

                        for (auto rhsitem : rhsitems)
                        {
                            if (rhsitem->getId() == "lit") {
                                auto singlit = (Literal*)rhsitem;
                                ofile << "\t\t\tstruct ptgast* node" << std::to_string(nodecount++) << ";\n";
                            }
                            else 
                            if (rhsitem->getId() == "tok") {
                                auto singlit = (Literal*)rhsitem;
                                ofile << "\t\t\tchar* tok" << std::to_string(tokcount++) << ";\n";
                            }
                        }

                        ofile << "\t\t} " << _astgroupnames[i] << ";\n";
                    }
                }
            }
        }

        /* we always increment state */
        state++;
    }

    templ.close();
    ofile.close();

    state = 0;
    templ.open("res/ptgparse.c.template");
    ofile.open("out/" + _fname + ".c");
    while (std::getline(templ, line))
    {
        /* if we aren't signalled to insert, leave it */
        if (line != delim) {
            ofile << line << "\n";
            continue;
        }

        /* otherwise... depends on state */
        switch (state)
        {
            case 0:
            {
                /* write include statement */
                ofile << "#include \"" << _fname << ".h\"\n";

                /* break state */
                break;
            }

            case 1:
            {
                /* use regular expressions */
                for (i = 0; i < _regexes.size(); ++i)
                {
                    /* get item */
                    auto item = _regexes[i];

                    /* get items and values */
                    name  = std::get<0>(item);
                    regex = std::get<1>(item);
                    subst = std::get<2>(item);

                    /* convert string to input string stream */
                    std::string innerline;
                    std::istringstream iss(regex);
                    
                    /* C code in regex should be printed line by line */
                    while (std::getline(iss, innerline)) {
                        ofile << "\t\t" << innerline << "\n";
                    }

                    /* return if positive, else keep going */
                    ofile << "\t\tif (load_bool()) {\n";
                    
                    /* handle, possibly, subset types */
                    if (subst.size() > 0) 
                    {
                        ofile << "\t\t\tjmp = sc->ptr;\n";

                        for (auto opt : subst)
                        {
                            ofile << "\t\t\tget_pos();\n";
                            ofile << "\t\t\tch = scan();\n";

                            iss.clear();
                            innerline.clear();
                            iss = std::istringstream(opt.second);

                            while (std::getline(iss, innerline)) {
                                ofile << "\t\t\t" << innerline << "\n";
                            } 
                            
                            ofile << "\t\t\tif (load_bool() && (jmp == sc->ptr))\n";
                            ofile << "\t\t\t\tptg_tokret(" << _elementtoks["#" + opt.first] << ")\n";
                        }
                        
                        ofile << "\t\t\tsc->ptr = jmp;\n";
                    }
                    
                    /* in any case, give the option of returning the suitable token */
                    ofile << "\t\t\tptg_tokret(" << _elementtoks["#" + name] << ")\n\t\t}\n";

                    /* behavior here varies depending on whether this is the last */
                    if (i < _regexes.size() - 1) {
                        ofile << "\t\telse {\n";
                        ofile << "\t\t\tget_pos();\n";
                        ofile << "\t\t\tch = scan();\n";
                        ofile << "\t\t}\n\n";
                    }
                }

                /* break state */
                break;
            }

            case 2:
            {
                /* add rule count */
                ofile << "#define P_RULE_COUNT "
                      << _hf->getRuleCount() << "\n";

                /* add rule count */
                ofile << "#define P_STATE_COUNT "
                      << _hf->getStateCount() << "\n";
                      
                /* add rule count */
                ofile << "#define P_ELEMENT_COUNT "
                      << _elements.size() << "\n";

                /* break state */
                break;
            }

            case 3:
            {
                /* declare transitions */

                for (auto x : _hf->getTransitions()) 
                {
                    auto a = x.first.first;
                    auto b = x.first.second;
                    auto c = x.second;

                    ofile << "\tptg_table_set(table, "
                          << a << ", " << _elementtoks[b]
                          << ", (ptg_tdata){ .action = ";

                    if (b.at(0) == '#') {
                        ofile << "PTGSHIFT, "
                              << ".op.shift = " 
                              << c << "});\n";
                    } else {
                        ofile << "PTGGOTO, "
                              << ".op.sgoto = " 
                              << c << "});\n";
                    }
                }

                /* get states, transitions, and follow-sets */
                auto states      = _hf->getStates();
                auto transitions = _hf->getTransitions();
                auto follow_set  = _hf->getFollowSet();

                for (i = 0; i < _hf->getStateCount(); ++i) 
                {
                    for (j = 0; j < states[i].size(); ++j) 
                    {
                        /* get handle fron state */
                        auto handle = states[i][j];

                        /* closed handles either ACCEPT or REDUCE */
                        if (handle.closed()) 
                        {
                            /* get lhs name */
                            auto rule      = handle.getRule();
                            auto rule_lhs  = rule->getLeft();
                            auto rule_rhs  = rule->getRight()->getChildren();
                            auto lhs_name  = rule_lhs->getName();
                            auto folwset   = rule->getFollow();
                            auto itemcount = 0;

                            /* if lhs is starting state, this is ACCEPT */
                            if (lhs_name == "S*") {
                                ofile << "\tptg_table_set(table, " 
                                      << i << ", " << _elementtoks["$"] 
                                      << ", (ptg_tdata){"
                                      << ".action = PTGACCEPT, .op.accept = 0});\n";
                                continue;
                            }
                            
                            /* find index of rule */
                            for (k = 0; k < _rules.size(); ++k)
                            {
                                Rule* itemk = _rules[k];
                                if ( rulecmp(itemk, rule) ) {

                                    if (rule_rhs.size() > 1 || !rule_rhs[0]->isEmpty()) {
                                        itemcount = rule->getRight()->getChildren().size();
                                    } else itemcount = 0;

                                    for (auto folw : folwset) {
                                        auto content = folw;
                                        if (folw != "$") content = "#" + content;
                                        ofile << "\tptg_table_set(table, " 
                                            << i << ", " << _elementtoks[content]
                                            << ", (ptg_tdata){"
                                            << ".action = PTGREDUCE, .op.reduce = {"
                                            << k
                                            << ", " << itemcount
                                            << ", " << _elementtoks[rule->getLeft()->getName()]
                                            << "}});\n";
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }

                /* break state */
                break;
            }

            case 4:
            {
                /* loop through all rules */
                for (i = 0; i < _rules.size(); ++i)
                {
                    auto rhsitems = _rules[i]->getRight()->getChildren();

                    /* write case statement */
                    ofile << "\t\t\t\t\tcase " << i << ":\n";
                    ofile << "\t\t\t\t\t{\n";

                    /* if empty statement, then simply add NULL to the stack */
                    if (rhsitems[0]->getId() == "empty") {
                        ofile << "\t\t\t\t\t\tnewast = NULL;\n";
                    }

                    else 
                    {
                        string gname;

                        /* find groupname of rule */
                        for (j = 0; j < _astgroups.size(); ++j) {
                            auto group = _astgroups[j];
                            for (k = 0; k < group.size(); ++k) {
                                if (i == group[k].first) {
                                    gname = _astgroupnames[j];
                                    break;
                                }
                            }
                        }

                        /* write gets for everything in rhs from nodestack IN REVERSE ORDER */
                        for (j = rhsitems.size() - 1; j >= 0; j--) {
                            ofile << "\t\t\t\t\t\tptgast* newast" << j << " = *(ptgast**)m_stack_pop(&nodes);\n";
                        }

                        /* aesthetic newline and newast type declaration */
                        ofile << "\t\t\t\t\t\tnewast->id = PTGAST_" << gname << ";\n";

                        /* if there is only one rhs item, it is stored as a singleton, if you recall */
                        if (rhsitems.size() == 1)
                        {
                            auto rhsitem = rhsitems[0];
                            if (rhsitem->getId() == "lit") {
                                ofile << "\t\t\t\t\t\tnewast->op." << gname << " = newast0;\n";
                            }
                            else if (rhsitem->getId() == "tok") {
                                ofile << "\t\t\t\t\t\tnewast->op." << gname << " = newast0->op.atom;\n";
                            }
                        }

                        else
                        {
                            /* store and maintain the tallies */
                            int tokcount  = 0;
                            int nodecount = 0;

                            /* construct newast by adding everything to its suitable place */
                            for (j = 0; j < rhsitems.size(); ++j) 
                            {
                                auto rhsitem = rhsitems[j];
                                
                                ofile << "\t\t\t\t\t\tnewast->op." << gname << ".";
                                if (rhsitem->getId() == "lit") {
                                    ofile << "node" << nodecount++ << " = newast" << j << ";\n";
                                }
                                else if (rhsitem->getId() == "tok") {
                                    ofile << "tok" << tokcount++ << " = newast" << j << "->op.atom;\n";
                                }
                            }
                        }
                    }

                    /* break statement */
                    ofile << "\t\t\t\t\t\tbreak;\n";

                    /* close case statement */
                    ofile << "\t\t\t\t\t}\n";

                    if (i < _rules.size() - 1) ofile << "\n";
                }

                /* break state */
                break;
            }

            case 5:
            {
                /* loop through all groupings */
                for (i = 0; i < _astgroups.size(); ++i) 
                {
                    auto groupitm = _astgroups[i][0].second;
                    auto rhsitems = groupitm->getRight()->getChildren();

                    /* don't bother with empties */
                    if (rhsitems.size() == 1 && rhsitems[0]->getId() == "empty") continue;

                    /* print out case statement */
                    ofile << "\t\t\tcase " << std::to_string(i) << ":\n\t\t\t{\n";

                    /* each node should first print out its name */
                    ofile << "\t\t\t\taddspacing(&str, INDENT);\n";
                    ofile << "\t\t\t\tm_string_concat(&str, \"" << _astgroupnames[i] << ":\\n\");\n\n";

                    /* if singleton, depends on whether item is char or ast* */
                    if (rhsitems.size() == 1) 
                    {
                        auto single = rhsitems[0];
                        
                        // if item is ast*, just get its own string representation and add it
                        if (single->getId() == "lit") 
                        {
                            ofile << "\t\t\t\ttmp = ptgast_str(ast->op.";
                            ofile << _astgroupnames[i] << ", INDENT + 1);\n";
                            ofile << "\t\t\t\tm_string_concat(&str, tmp);\n\n";
                        }
                        // otherwise, just add token name
                        else 
                        if (single->getId() == "tok") 
                        {
                            ofile << "\t\t\t\tif (ast->op.";
                            ofile << _astgroupnames[i] << ") {\n";
                            ofile << "\t\t\t\t\taddspacing(&str, INDENT + 1);\n";
                            ofile << "\t\t\t\t\tm_string_concat(&str, ast->op.";
                            ofile << _astgroupnames[i] << ");\n";
                            ofile << "\t\t\t\t\tm_string_push(&str, '\\n');\n\t\t\t\t}\n\n";
                        }
                    }

                    /* much of the same thing, though use tokcount and nodecount here */
                    else 
                    {
                        int tokcount  = 0;
                        int nodecount = 0;

                        for (k = 0; k < rhsitems.size(); ++k)
                        {
                            auto rhsitem = rhsitems[k];

                            if (rhsitem->getId() == "lit") 
                            {
                                ofile << "\t\t\t\ttmp = ptgast_str(ast->op.";
                                ofile << _astgroupnames[i] << ".node";
                                ofile << std::to_string(nodecount++) << ", INDENT + 1);\n";
                                ofile << "\t\t\t\tm_string_concat(&str, tmp);\n\n";
                            }
                            else 
                            if (rhsitem->getId() == "tok") 
                            {
                                ofile << "\t\t\t\tif (ast->op.";
                                ofile << _astgroupnames[i] << ".tok";
                                ofile << std::to_string(tokcount) << ") {\n";
                                ofile << "\t\t\t\t\taddspacing(&str, INDENT + 1);\n";
                                ofile << "\t\t\t\t\tm_string_concat(&str, ast->op.";
                                ofile << _astgroupnames[i] << ".tok";
                                ofile << std::to_string(tokcount++) << ");\n";
                                ofile << "\t\t\t\t\tm_string_push(&str, '\\n');\n\t\t\t\t}\n\n";
                            }
                        }
                    }

                    /* exit case statement */
                    ofile << "\t\t\t\tbreak;\n\t\t\t}\n";
                }

                /* break state */
                break;
            }
        }

        /* we always increment state */
        state++;
    }

    templ.close();
    ofile.close();
}