#include "../include/parser/parser.hpp"

// Parser operators

void Parser::ppush(ParserItem* p) {
    pred_stack.push_back(p);
}

ParserItem* Parser::ppop() {
    ParserItem *p = pred_stack.back();
    pred_stack.pop_back();
    return p;
}

void Parser::pprint() {
    if (!pred_stack.empty()) {
        for (int k = pred_stack.size() - 1; k >= 0; --k) {
            cout << "name: " << tokname(pred_stack[k]->name())
                << ", type: " << pred_stack[k]->type() 
                << std::endl;
        }
        cout << std::endl;
    }
}

// RegEx operators

void Parser::reg_push(string a) {
    reg_stack.push_back(a);
}

string Parser::reg_pop() {
    string a = reg_stack.back();
    reg_stack.pop_back();
    return a;
}

void Parser::reg_print() {
    /* if (!res_stack.empty()) {
        for (int k = (int)res_stack.size() - 1; k >= 0; --k) {
            cout << "val[" << k << "]: " 
                    << res_stack[k]->getId()
                    << std::endl;
        }
        cout << std::endl;
    } */
}

// AST operators

void Parser::ast_push(AST* a) {
    res_stack.push_back(a);
}

AST* Parser::ast_pop() {
    AST* a = res_stack.back();
    res_stack.pop_back();
    return a;
}

void Parser::ast_print() {
    if (!res_stack.empty()) {
        for (int k = (int)res_stack.size() - 1; k >= 0; --k) {
            cout << "val[" << k << "]: " 
                    << res_stack[k]->getId()
                    << std::endl;
        }
        cout << std::endl;
    }
}

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

void Parser::print_root() {
    if (!root) {
        parse_err("cannot view root before parsing\n");
    } else root->print();
}

void Parser::print_dict() {
    ParseDict::iterator it;
    for (it = dict.begin(); it != dict.end(); ++it) {
        cout << "(" << it->first.first << ", "
                << it->first.second << ": [";
        for (auto x : *(it->second)) {
            cout << x << ", ";
        } cout << "]\n";
    }
}

int Parser::prime_table() {
    dict[make_pair<Tokens, Tokens>(M_START, S_STRING)] = new vector<Tokens>({S_START, M_START1});
    dict[make_pair<Tokens, Tokens>(M_START, S_NEWLINE)] = new vector<Tokens>({S_START, M_START1});
    dict[make_pair<Tokens, Tokens>(M_START, S_DELIM)] = new vector<Tokens>({S_START, M_START1});
    dict[make_pair<Tokens, Tokens>(M_START, ENDFILE)] = new vector<Tokens>({S_START, M_START1});
    dict[make_pair<Tokens, Tokens>(M_START1, S_DELIM)] = new vector<Tokens>({S_DELIM, P_START});
    dict[make_pair<Tokens, Tokens>(M_START1, ENDFILE)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(S_START, S_STRING)] = new vector<Tokens>({S_RULE, S_START});
    dict[make_pair<Tokens, Tokens>(S_START, S_NEWLINE)] = new vector<Tokens>({S_RULE, S_START});
    dict[make_pair<Tokens, Tokens>(S_START, S_DELIM)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(S_START, ENDFILE)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(S_RULE, S_STRING)] = new vector<Tokens>({S_STRING, S_TRANSIT, S_CONTENT, S_NEWLINE});
    dict[make_pair<Tokens, Tokens>(S_RULE, S_NEWLINE)] = new vector<Tokens>({S_NEWLINE});
    dict[make_pair<Tokens, Tokens>(P_START, ENDFILE)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(P_START, RULE)] = new vector<Tokens>({P_RULE, P_START});
    dict[make_pair<Tokens, Tokens>(P_START, BREAK)] = new vector<Tokens>({P_RULE, P_START});
    dict[make_pair<Tokens, Tokens>(P_RULE, RULE)] = new vector<Tokens>({RULE, ARROW, P_RULES, BREAK});
    dict[make_pair<Tokens, Tokens>(P_RULE, BREAK)] = new vector<Tokens>({BREAK});
    dict[make_pair<Tokens, Tokens>(P_RULES, EMPTY)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES, RULE)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    //dict[make_pair<Tokens, Tokens>(P_RULES, TOK)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES, LOPT)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES, LREP)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, EMPTY)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, RULE)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    //dict[make_pair<Tokens, Tokens>(P_RULES1, TOK)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, LOPT)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, LREP)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, BAR)] = new vector<Tokens>({BAR, P_RULES_EL, P_RULES1});
    dict[make_pair<Tokens, Tokens>(P_RULES1, ROPT)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(P_RULES1, RREP)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(P_RULES1, BREAK)] = new vector<Tokens>({});
    dict[make_pair<Tokens, Tokens>(P_RULES_EL, EMPTY)] = new vector<Tokens>({EMPTY});
    dict[make_pair<Tokens, Tokens>(P_RULES_EL, RULE)] = new vector<Tokens>({RULE});
    //dict[make_pair<Tokens, Tokens>(P_RULES_EL, TOK)] = new vector<Tokens>({TOK});
    dict[make_pair<Tokens, Tokens>(P_RULES_EL, LOPT)] = new vector<Tokens>({LOPT, P_RULES, ROPT});
    dict[make_pair<Tokens, Tokens>(P_RULES_EL, LREP)] = new vector<Tokens>({LREP, P_RULES, RREP});
    return EXIT_SUCCESS;
}

int Parser::parse() {
    ppush(new ParserTok(ENDFILE));
    ppush(new ParserRule(M_START));

    ParserItem *tos;
    Tokens cur = sc->lex();
    if (_flags.SCANNER_TRACE)
        cout << "first symbol: " 
                << tokname(cur) 
                << std::endl;

    vector<AST> trash_pile = {};
    
    while (!pred_stack.empty()) {
        
        if (_flags.PARSER_TRACE) 
            pprint();
        tos = pred_stack.back();
        
        if (tos->type() == "reduce") {
            ParserRed *mytos = (ParserRed*)tos;
            tos = ppop();
            int len = mytos->getarg_count();
            
            AST* head;
            vector<AST*> tail;
            for (int i = 0; i < len; i++) {
                tail.push_back(ast_pop());
            }
            
            switch(mytos->name()) {
                case Tokens::P_START: {
                    if (len == 0)
                        ast_push(new StartAST());
                    else
                    if (len == 2) {
                        StartAST *start = (StartAST*)tail[0];
                        if (tail[1]->getId() != "empty")
                            start->add(tail[1]);
                        else delete tail[1];
                        ast_push(tail[0]);
                    }
                    break;
                }

                case Tokens::P_RULE: {
                    if (len == 4)
                        ast_push(new Rule((Literal*)tail[3], (RuleList*)tail[1]));
                    else
                    if (len == 1)
                        ast_push(new EmptyAST());
                    break;
                }
                
                case Tokens::P_RULES: {
                    auto x = (RuleList*)tail[0];
                    if (x->isEmpty() || !x->curr_is_or_node()) {
                        x->addChild(tail[1]);
                        ast_push(tail[0]);
                    }
                    else {
                        auto b = (OrExpr*)x->last();
                        b->addLeft(tail[1]);
                        ast_push(tail[0]);
                    }
                    break;
                }
                
                case Tokens::P_RULES1: {
                    if (len == 0) {
                        ast_push(new RuleList());
                    }
                    else
                    if (len == 2) {
                        auto x = (RuleList*)tail[0];
                        if (x->isEmpty() || !x->curr_is_or_node()) {
                            x->addChild(tail[1]);
                            ast_push(tail[0]);
                        }
                        else {
                            auto b = (OrExpr*)x->last();
                            b->addLeft(tail[1]);
                            ast_push(tail[0]);
                        }
                    }
                    else
                    if (len == 3) {
                        auto x = (RuleList*)tail[0];
                        if (x->isEmpty() || !x->curr_is_or_node()) {
                            x->addChild(tail[1]);
                        }
                        else {
                            auto b = (OrExpr*)x->last();
                            b->addLeft(tail[1]);
                        } ast_push(new RuleList(new OrExpr(x)));
                    }
                    break;
                }
                
                case Tokens::P_RULES_EL: {
                    if (len == 1)
                        ast_push(tail[0]);
                    else {
                        auto tok = ((Literal*)tail[0])->getToken();
                        if (tok == Tokens::ROPT)
                            ast_push(new OptExpr((RuleList*)tail[1]));
                        else
                            ast_push(new RepExpr((RuleList*)tail[1]));
                    }
                    break;
                }

                case Tokens::M_START: {
                    ast_push(tail[1]);
                    ast_push(tail[0]);
                    break;
                }

                case Tokens::M_START1: {
                    if (len == 2) {
                        delete tail[1];
                        ast_push(tail[0]);
                    } else ast_push(new EmptyAST());
                    break;
                }

                case Tokens::S_RULE: {
                    if (len == 4) {
                        auto str = (Literal*)tail[3];
                        auto regex = (Literal*)tail[1];
                        delete tail[2];
                        delete tail[0];
                        ast_push(new RegRule(str->getName(), regex->getName()));
                    } else ast_push(new EmptyAST());
                    break;
                }

                case Tokens::S_START: {
                    if (len == 2) {
                        StartAST *start = (StartAST*)tail[0];
                        if (tail[1]->getId() != "empty")
                            start->add(tail[1]);
                        else delete tail[1];
                        ast_push(tail[0]);
                    } else ast_push(new StartAST());
                    break;
                }
                
                default: {
                    std::string err = "invalid reduce symbol ";
                    err += tokname(mytos->name());
                    parse_err(err.c_str());
                }
            }

            if (_flags.PARSER_TRACE)
                ast_print();
        }

        else
        if (tos->type() == "nonterm") {
            ParsePair x = make_pair<Tokens, Tokens>((Tokens)tos->name(), (Tokens)cur);
            if (dict.count(x) == 0)
                parse_illegal_transition_err((Tokens)tos->name(), (Tokens)cur);
            else {
                tos = ppop();
                auto x = dict[make_pair<Tokens, Tokens>((Tokens)tos->name(), (Tokens)cur)];
                int len = x->size();
                ppush(new ParserRed(tos->name(), len));
                if (len > 0) {
                    for (int i = len - 1; i >= 0; i--)
                        if (is_terminal((*x)[i])) ppush(new ParserTok((*x)[i])); else ppush(new ParserRule((*x)[i]));
                }
                free(tos);         
            }
        }

        else {
            if (tos->name() == cur) {
                tos = ppop();
                if (cur != Tokens::ENDFILE) {
                    if (cur == Tokens::EMPTY)
                        ast_push(new EmptyAST());
                    else 
                    if (!is_parser_enum(cur) || cur != Tokens::RULE)
                        ast_push(new Literal(sc->getlexeme(), cur));
                    else
                    {
                        auto toks = ((StartAST*)res_stack[0])->getChildren();
                        bool _f = false;
                        for (auto x : toks) {
                            auto r = (RegRule*)x;
                            if (r->getName() == sc->getlexeme()) {
                                _f = true;
                                break;
                            }
                        }
                        if (_f)
                            ast_push(new Literal(sc->getlexeme(), Tokens::TOK));
                        else ast_push(new Literal(sc->getlexeme(), Tokens::RULE));
                    }

                    if (_flags.PARSER_TRACE)
                        ast_print();
                }
                cur = sc->lex();
                if (_flags.SCANNER_TRACE)
                    cout << "next symbol: " 
                            << tokname(cur) 
                            << std::endl;
                free(tos);
            } else parse_unexpected_terminal_err((Tokens)tos->name(), (Tokens)cur);
        }
    }

    root    = (StartAST*)res_stack[1];
    regexes = (StartAST*)res_stack[0];
    return EXIT_SUCCESS;
}