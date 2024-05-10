#include "../include/parser/parser.hpp"

// warnings and errors

void Parser::parse_warn(const char* ch) {
    cerr << "parser: " << ch << " at line " << sc->getlineno() << std::endl;
}

void Parser::parse_err(const char* ch) {
    parse_warn(ch);
    exit(-1);
}

void Parser::parse_unexpected_terminal_err(Tokens t_exp, Tokens t_rec) {
    string ch = "expected " + string(tokname(t_exp)) + ", got " + string(tokname(t_rec)) + "(" + sc->getlexeme() + ")";
    parse_err(ch.c_str());
}

void Parser::parse_illegal_transition_err(Tokens t_tos, Tokens t_cur) {
    string ch = "cannot transitiion from " + string(tokname(t_tos)) + " with input " + string(tokname(t_cur));
    parse_err(ch.c_str());
}

bool Parser::sc_exists(string name)
{
    for (auto scitem : _scitems) {
        if (scitem.first == name)
            return true;
    } return false;
}

void Parser::parse()
{
    sc_parse();
    if (sc->sdir().state == 1) {
        pr_parse();
    }
}

void Parser::sc_parse()
{
    pitem tos;
    Tokens a;
    ParserTable tb;
    vector<reglit> nodes;
    vector<pitem> stack;

    tb[{START, ENDFILE}] = {};
    tb[{START, STRING}] = vector<Tokens>{RULE, START};
    tb[{START, NEWLINE}] = vector<Tokens>{RULE, START};
    tb[{RULE, STRING}] = vector{STRING, STRING, NEWLINE};
    tb[{RULE, NEWLINE}] = vector<Tokens>{NEWLINE};

    stack.push_back(pitem{.id = pitem::LIT, .op = {.lit = ENDFILE}});
    stack.push_back(pitem{.id = pitem::NONLIT, .op = {.nonlit = START}});

    a = sc->lex();
    if (_flags.PARSER_TRACE)
        cout << "a = " << tokname(a) << ".\n";

    while (!stack.empty())
    {
        tos = stack.back();
        stack.pop_back();
        if (_flags.PARSER_TRACE) {
            cout << "\nnew iteration:\n";
            cout << "  stack.pop() = " << pitem_string(tos) << ".\n";
        }

        switch (tos.id)
        {
            case pitem::LIT:
                if (tos.op.lit == a) {
                    auto lexs  = sc->getlexeme();
                    nodes.push_back({"", lexs});
                    if (_flags.PARSER_TRACE)
                        cout << "  terms.push(\"\", " << tokname(a) << ").\n";
                    a = sc->lex();
                    if (_flags.PARSER_TRACE)
                        cout << "  a = " << tokname(a) << ".\n";
                } else parse_unexpected_terminal_err(tos.op.lit, a);
                break;

            case pitem::NONLIT:
                if (tb.find({tos.op.nonlit, a}) == tb.end()) {
                    parse_illegal_transition_err(tos.op.nonlit, a);
                } else {
                    auto res = tb[{tos.op.nonlit, a}];
                    if (_flags.PARSER_TRACE)
                        cout << "  table[" << tokname(tos.op.nonlit) << ", " << tokname(a) << "] = " << pitem_string(tos) << ".\n";
                    stack.push_back(pitem{.id = pitem::REDUCTION, .op = {.reduce = {.tag = tos.op.nonlit, .count = res.size()}}});
                    for (int i = res.size(); i > 0; i--) {
                        auto item = res[i-1];
                        if (isterminal(item)) {
                            auto pt = pitem{.id = pitem::LIT, .op = {.lit = item}};
                            stack.push_back(pt);
                            if (_flags.PARSER_TRACE)
                                cout << "  stack.push(" << pitem_string(pt) << ").\n";
                        } else {
                            auto pt = pitem{.id = pitem::NONLIT, .op = {.nonlit = item}};
                            stack.push_back(pt);
                            if (_flags.PARSER_TRACE)
                                cout << "  stack.push(" << pitem_string(pt) << ").\n";
                        }
                    }
                }
                break;

            case pitem::REDUCTION:
                switch (tos.op.reduce.tag)
                {
                    case START:
                    {
                        if (tos.op.reduce.count == 0) {
                            nodes.push_back({"", ""});
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push({}).\n";
                        }
                        else
                        if (tos.op.reduce.count == 2) 
                        {
                            nodes.pop_back();
                            reglit b = nodes.back();  // Rule
                            nodes.pop_back();
                            if (!b.first.empty()) {
                                _scitems.push_front(b);
                                if (_flags.PARSER_TRACE)
                                    cout << "  answer.push({" << b.first << ", " << b.second << "}).\n";
                            }
                            nodes.push_back({"", ""});
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push({}).\n";
                        }
                        break;
                    }

                    case RULE:
                    {
                        if (tos.op.reduce.count == 3)
                        {
                            nodes.pop_back();
                            reglit a = nodes.back();  // STRING
                            nodes.pop_back();
                            reglit b = nodes.back();  // STRING
                            nodes.pop_back();
                            nodes.push_back({b.second, a.second});
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push({" << b.second << ", " << a.second << "}).\n";
                        }
                        else
                        if (tos.op.reduce.count == 1) {
                            nodes.pop_back();
                            nodes.push_back({"", ""});
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push({}).\n";
                        }
                    }
                }
        }
    } 
    
    sc->unlex(a);
}

void Parser::pr_parse()
{
    pitem tos;
    Tokens a;
    ParserTable tb;
    vector<AST*> nodes;
    vector<pitem> stack;

    tb[{START, ENDFILE}] = vector<Tokens>();
    tb[{START, LIT}] = vector{RULE, START};
    //tb[{START, BREAK}] = vector{RULE, START};
    tb[{RULE, LIT}] = vector{LIT, ARROW, RULES, BREAK};
    //tb[{RULE, BREAK}] = vector{BREAK};
    tb[{RULES, LIT}] = vector{RULESEL, RULES1};
    tb[{RULES, EMPTY}] = vector{RULESEL, RULES1};
    tb[{RULES, LOPT}] = vector{RULESEL, RULES1};
    tb[{RULES, LREP}] = vector{RULESEL, RULES1};
    tb[{RULES1, BAR}] = vector{BAR, RULESEL, RULES1};
    tb[{RULES1, LIT}] = vector{RULESEL, RULES1};
    tb[{RULES1, EMPTY}] = vector{RULESEL, RULES1};
    tb[{RULES1, LOPT}] = vector{RULESEL, RULES1};
    tb[{RULES1, LREP}] = vector{RULESEL, RULES1};
    tb[{RULES1, BREAK}] = vector<Tokens>{};
    tb[{RULES1, ROPT}] = vector<Tokens>{};
    tb[{RULES1, RREP}] = vector<Tokens>{};
    tb[{RULESEL, LIT}] = vector{LIT};
    tb[{RULESEL, EMPTY}] = vector{EMPTY};
    tb[{RULESEL, LOPT}] = vector{LOPT, RULES, ROPT};
    tb[{RULESEL, LREP}] = vector{LREP, RULES, RREP};

    stack.push_back(pitem{.id = pitem::LIT, .op = {.nonlit = ENDFILE}});
    stack.push_back(pitem{.id = pitem::NONLIT, .op = {.nonlit = START}});

    a = sc->lex();
    if (_flags.PARSER_TRACE)
        cout << "a = " << tokname(a) << ".\n";

    while (!stack.empty())
    {
        tos = stack.back();
        stack.pop_back();
        if (_flags.PARSER_TRACE) {
            cout << "\nnew iteration:\n";
            cout << "  stack.pop() = " << pitem_string(tos) << ".\n";
        }

        switch (tos.id)
        {
            case pitem::LIT:
                if (tos.op.lit == a) 
                {   
                    if (a == Tokens::LIT) {
                        auto lexs  = sc->getlexeme();
                        bool isTok = sc_exists(lexs);
                        nodes.push_back(new Literal(lexs, isTok ? Tokens::TOK : Tokens::LIT));
                        if (_flags.PARSER_TRACE)
                            cout << "  terms.push(" << tokname(isTok ? Tokens::TOK : Tokens::LIT) << ").\n";
                    } else {
                        if (a == Tokens::EMPTY) {
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push($).\n";
                            nodes.push_back(new EmptyAST());
                        } else {
                            if (_flags.PARSER_TRACE)
                                cout << "  terms.push(" << tokname(a) << ").\n";
                            nodes.push_back(new Literal("", a));
                        }
                    } 
                    a = sc->lex();
                    if (_flags.PARSER_TRACE)
                        cout << "  a = " << tokname(a) << ".\n";
                } else parse_unexpected_terminal_err(tos.op.lit, a);
                break;

            case pitem::NONLIT:
                if (tb.find({tos.op.nonlit, a}) == tb.end()) {
                    parse_illegal_transition_err(tos.op.lit, a);
                } else {
                    auto res = tb[{tos.op.nonlit, a}];
                    if (_flags.PARSER_TRACE)
                        cout << "  table[" << tokname(tos.op.nonlit) << ", " << tokname(a) << "] = " << pitem_string(tos) << ".\n";
                    stack.push_back(pitem{.id = pitem::REDUCTION, .op = {.reduce = {.tag = tos.op.nonlit, .count = res.size()}}});
                    for (int i = res.size(); i > 0; i--) {
                        auto item = res[i-1];
                        if (isterminal(item)) {
                            auto pt = pitem{.id = pitem::LIT, .op = {.lit = item}};
                            stack.push_back(pt);
                            if (_flags.PARSER_TRACE)
                                cout << "  stack.push(" << pitem_string(pt) << ").\n";
                        } else {
                            auto pt = pitem{.id = pitem::NONLIT, .op = {.nonlit = item}};
                            stack.push_back(pt);
                            if (_flags.PARSER_TRACE)
                                cout << "  stack.push(" << pitem_string(pt) << ").\n";
                        }
                    }
                }
                break;

            case pitem::REDUCTION:
                switch (tos.op.reduce.tag)
                {
                    case START:
                    {
                        if (tos.op.reduce.count == 0) {
                            if (_flags.PARSER_TRACE)
                                cout << "  Start <- empty.\n";
                            nodes.push_back(new EmptyAST());
                        }
                        else
                        if (tos.op.reduce.count == 2)
                        {  
                            if (_flags.PARSER_TRACE)
                                cout << "  Start <- Rule Start.\n";

                            AST* a = nodes.back();  // Start
                            nodes.pop_back();
                            AST* b = nodes.back();  // Start
                            nodes.pop_back();

                            if (b->getId() != "empty") {
                                _psitems.push_front((Rule*)b);
                                if (_flags.PARSER_TRACE) {
                                    cout << "  pushed to results:\n";
                                    ((Rule*)b)->print(2);
                                }
                            }
                            
                            nodes.push_back(new EmptyAST());
                        }

                        break;
                    }

                    case RULE:
                    {
                        if (tos.op.reduce.count == 4)
                        {   
                            nodes.pop_back();
                            AST* a = nodes.back();  // Rules
                            nodes.pop_back();
                            nodes.pop_back();
                            AST* b = nodes.back();  // LIT
                            nodes.pop_back();

                            if (_flags.PARSER_TRACE)
                                cout << "  Rule <- LIT ARROW Rules BREAK.\n";

                            RuleList* l;
                            auto orw = (OrExpr*)a;

                            if (orw->emptyleft()) {
                                nodes.push_back(new Rule((Literal*)b, new RuleList(orw->getRight())));
                            } else if (orw->emptyright()) {
                                nodes.push_back(new Rule((Literal*)b, new RuleList(orw->getLeft())));
                            } else nodes.push_back(new Rule((Literal*)b, new RuleList(a)));

                        }
                        else
                        if (tos.op.reduce.count == 1) {
                            nodes.pop_back();
                            if (_flags.PARSER_TRACE)
                                cout << "  Rule <- BREAK.\n";
                            nodes.push_back(new EmptyAST());
                        }

                        break;
                    }

                    case RULES:
                    {   
                        AST* a = nodes.back();  // Rules'
                        nodes.pop_back();
                        AST* b = nodes.back();  // RulesEl
                        nodes.pop_back();

                        if (_flags.PARSER_TRACE)
                            cout << "  Rules <- RulesEl Rules'.\n";

                        auto orw = (OrExpr*)a;
                        orw->add(b);
                        nodes.push_back(a);

                        break;
                    }

                    case RULES1:
                    {   
                        if (tos.op.reduce.count == 0) {
                            if (_flags.PARSER_TRACE)
                                cout << "  Rules' <- empty.\n";
                            nodes.push_back(new OrExpr());
                        }
                        else
                        if (tos.op.reduce.count == 2) 
                        {
                            AST* a = nodes.back();  // Rules'
                            nodes.pop_back();
                            AST* b = nodes.back();  // RulesEl
                            nodes.pop_back();

                            if (_flags.PARSER_TRACE)
                                cout << "  Rules' <- RulesEl Rules'.\n";

                            auto orw = (OrExpr*)a;
                            orw->add(b);
                            nodes.push_back(a);
                        }
                        else
                        if (tos.op.reduce.count == 3)
                        {
                            AST* a = nodes.back();  // Rules'
                            nodes.pop_back();
                            AST* b = nodes.back();  // RulesEl
                            nodes.pop_back();
                            nodes.pop_back();

                            if (_flags.PARSER_TRACE)
                                cout << "  Rules' <- OR RulesEl Rules'.\n";

                            auto orw = (OrExpr*)a;
                            orw->add(b);
                            orw->swap();
                            nodes.push_back(a);
                        }

                        break;
                    }

                    case RULESEL:
                    {
                        if (tos.op.reduce.count == 1) {
                            AST* a = nodes.back();
                            nodes.pop_back();
                            auto tok = ((Literal*)a)->getToken();
                            if (_flags.PARSER_TRACE) {
                                if (tok == Tokens::LIT || tok == Tokens::TOK) 
                                    cout << "  RulesEl <- LIT.\n";
                                else cout << "  RulesEl <- EMPTY.\n";
                            }
                            nodes.push_back(a);
                        }
                        else
                        if (tos.op.reduce.count == 3) 
                        {
                            AST* a = nodes.back();  // ROPT/RREP
                            nodes.pop_back();
                            AST* b = nodes.back();  // Rules
                            nodes.pop_back();
                            nodes.pop_back();

                            auto tok = ((Literal*)a)->getToken();
                            auto orw = (OrExpr*)b;
                            deque<AST*> l = orw->emptyleft() ? orw->getRight() : (orw->emptyright() ? orw->getLeft() : deque{b});

                            if (tok == Tokens::ROPT) {
                                if (_flags.PARSER_TRACE)
                                    cout << "  RulesEl <- LOPT Rules ROPT.\n";
                                nodes.push_back(new OptExpr(new RuleList(l)));
                            } else {
                                if (_flags.PARSER_TRACE)
                                    cout << "  RulesEl <- LREP Rules RREP.\n";
                                nodes.push_back(new RepExpr(new RuleList(l)));
                            }
                        }

                        break;
                    }
                }
        }
    }

    sc->unlex(a);
}