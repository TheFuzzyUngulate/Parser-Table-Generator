#ifndef PARSER_HPP
#define PARSER_HPP

#pragma once

#include "scanner.hpp"
#include "parseritems.hpp"
#include "ast.hpp"
#include "runutils.hpp"

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

class Parser {
    public:
        Parser(Scanner* scptr) {
            sc = scptr;
            root = new ast_rt();
            prime_table();
        }

        // push into the prediction stack
        void ppush(ParserItem* p) {pred_stack.push_back(p);}
        // pop ParserItem from the prediction stack
        ParserItem* ppop() {ParserItem *p = pred_stack.back(); pred_stack.pop_back(); return p;}
        // print prediction stack
        void pprint() {
            if (!pred_stack.empty()) {
                for (int k = pred_stack.size() - 1; k >= 0; --k) {
                    cout << "name: " << tokname(pred_stack[k]->name())
                        << ", type: " << pred_stack[k]->type() 
                        << std::endl;
                }
                cout << std::endl;
            }
        }
        // push into the ast stack
        void apush(ast *a) {res_stack.push_back(a);}
        // pop ast from the ast stack
        ast* apop() {ast *a = res_stack.back(); res_stack.pop_back(); return a;}
        // print ast stack
        void aprint() {
            if (!res_stack.empty()) {
                for (int k = res_stack.size() - 1; k >= 0; --k) {
                    if (res_stack[k]->get_tok() >= 0) {
                        string nom = tokname(res_stack[k]->get_tok());
                        cout << "val[" << k << "]: " 
                             << res_stack[k]->get_type() 
                             << ((res_stack[k]->get_type() == "lit") 
                                ? string(", tok: ") + tokname(res_stack[k]->get_tok()) : "")
                             << std::endl;
                    } else cout << "aprint: empty stmt\n";
                }
                cout << std::endl;
            }
        }
        // check if object is a terminal or non-terminal
        bool is_terminal(Tokens t) {return t < P_START;}
        // parser warning
        void parse_warn(const char* ch) {cerr << "parser: " << ch << " at line " << sc->getlineno() << std::endl;}
        // general parser error
        void parse_err(const char* ch) {parse_warn(ch); exit(-1);}
        // unexpected terminal error (special case)
        void parse_unexpected_terminal_err(Tokens t_exp, Tokens t_rec) {
            string ch = "expected " + string(tokname(t_exp)) + ", got " + string(tokname(t_rec)) + "(" + sc->getlexeme() + ")";
            parse_err(ch.c_str());
        }
        void parse_illegal_transition_err(Tokens t_tos, Tokens t_cur) {
            string ch = "cannot transitiion from " + string(tokname(t_tos)) + " with input " + string(tokname(t_cur));
            parse_err(ch.c_str());
        }
        
        void print_root() {
            if (root == nullptr) {
                parse_err("cannot view root before parsing\n");
            } else root->print();
        }
        ast_rt* getroot() {return root;}

        int prime_table() {
            dict[make_pair<Tokens, Tokens>(P_START, ENDFILE)] = new vector<Tokens>({});
            dict[make_pair<Tokens, Tokens>(P_START, RULE)] = new vector<Tokens>({P_RULE, P_START});
            dict[make_pair<Tokens, Tokens>(P_START, BREAK)] = new vector<Tokens>({P_RULE, P_START});
            dict[make_pair<Tokens, Tokens>(P_RULE, RULE)] = new vector<Tokens>({RULE, ARROW, P_RULES, BREAK});
            dict[make_pair<Tokens, Tokens>(P_RULE, BREAK)] = new vector<Tokens>({BREAK});
            dict[make_pair<Tokens, Tokens>(P_RULES, EMPTY)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES, RULE)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES, TOK)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES, LOPT)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES, LREP)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, EMPTY)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, RULE)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, TOK)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, LOPT)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, LREP)] = new vector<Tokens>({P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, BAR)] = new vector<Tokens>({BAR, P_RULES_EL, P_RULES1});
            dict[make_pair<Tokens, Tokens>(P_RULES1, ROPT)] = new vector<Tokens>({});
            dict[make_pair<Tokens, Tokens>(P_RULES1, RREP)] = new vector<Tokens>({});
            dict[make_pair<Tokens, Tokens>(P_RULES1, BREAK)] = new vector<Tokens>({});
            dict[make_pair<Tokens, Tokens>(P_RULES_EL, EMPTY)] = new vector<Tokens>({EMPTY});
            dict[make_pair<Tokens, Tokens>(P_RULES_EL, RULE)] = new vector<Tokens>({RULE});
            dict[make_pair<Tokens, Tokens>(P_RULES_EL, TOK)] = new vector<Tokens>({TOK});
            dict[make_pair<Tokens, Tokens>(P_RULES_EL, LOPT)] = new vector<Tokens>({LOPT, P_RULES, ROPT});
            dict[make_pair<Tokens, Tokens>(P_RULES_EL, LREP)] = new vector<Tokens>({LREP, P_RULES, RREP});
            return EXIT_SUCCESS;
        }

        void print_dict() {
            ParseDict::iterator it;
            for (it = dict.begin(); it != dict.end(); ++it) {
                cout << "(" << it->first.first << ", "
                     << it->first.second << ": [";
                for (auto x : *(it->second)) {
                    cout << x << ", ";
                } cout << "]\n";
            }
        }

        int parse() {
            ppush(new ParserTok(ENDFILE));
            ppush(new ParserRule(P_START));

            ParserItem *tos;
            int cur = sc->lex();
            if (flags.SCANNER_TRACE)
                cout << "first symbol: " 
                     << tokname(cur) 
                     << std::endl;
            while (!pred_stack.empty()) {
                if (flags.PARSER_TRACE) 
                    pprint();
                tos = pred_stack.back();
                if (tos->type() == "reduce") {
                    ParserRed *mytos = (ParserRed*)tos;
                    tos = ppop();
                    int len = mytos->getarg_count();
                    
                    ast *head;
                    vector<ast*> tail;
                    for (int i = 0; i < len; i++)
                        tail.push_back(apop());

                    switch(mytos->name()) {
                        case Tokens::P_START: {
                            if (len == 0)
                                head = new ast_rt();
                            else {
                                head = (ast_rt*)tail[0];
                                if (tail[1]->get_tok() > -1)
                                    head->add(tail[1]);
                            } break;
                        }
                        
                        case Tokens::P_RULE: {
                            if (len == 1)
                                head = new ast();
                            else {
                                vector<ast*> lst;
                                ast_el* root = (ast_el*)tail[1];
                                while (root != nullptr) {
                                    lst.push_back(root);
                                    root = (ast_el*)root->get_nxt();
                                } head = new ast_rule(tail[3], lst);
                            } break;
                        }
                        
                        case Tokens::P_RULES: {
                            if (tail[0]->get_type() == "or-stmt") {
                                ast_or *myr = (ast_or*)((ast_el*)tail[0])->get_ast();
                                myr->setleft(tail[1]);
                                head = tail[0];
                            }
                            else
                            if (tail[0]->get_tok() == -1) {
                                head = tail[1];
                            }
                            else {
                                ast_el *myr = (ast_el*)tail[1];
                                myr->addnext(tail[0]);
                                head = myr;
                            } break;
                        }
                        
                        case Tokens::P_RULES1: {
                            if (len == 0)
                                head = new ast();
                            else
                            if (len == 3) {
                                if (tail[0]->get_type() == "or-stmt") {
                                    ast_or *myr = (ast_or*)((ast_el*)tail[0])->get_ast();
                                    myr->setleft(tail[1]);
                                    head = new ast_el(new ast_or(myr));
                                }
                                else {
                                    ast_el *myr = (ast_el*)tail[1];
                                    myr->addnext(tail[0]);
                                    head = new ast_el(new ast_or(myr));
                                }
                            } 
                            else 
                            if (len == 2) {
                                if (tail[0]->get_type() == "or-stmt") {
                                    ast_or *myr = (ast_or*)((ast_el*)tail[0])->get_ast();
                                    myr->setleft(tail[1]);
                                    head = tail[0];
                                }
                                else {
                                    ast_el *myr = (ast_el*)tail[1];
                                    myr->addnext(tail[0]);
                                    head = myr;
                                }
                            } break;
                        }
                        
                        case Tokens::P_RULES_EL: {
                            if (len == 1)
                                head = new ast_el(tail[0]);
                            else {
                                ast_el *myop = (ast_el*)tail[1];
                                head = new ast_el(new ast_in(myop, (Tokens)tail[0]->get_tok()));
                            } 
                            break;
                        }

                        default: 
                            parse_err("invalid reduce symbol");
                    }
                    
                    apush(head);
                    if (flags.PARSER_TRACE)
                        aprint();
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
                        if (len > 0)
                            for (int i = len - 1; i >= 0; i--)
                                if (is_terminal((*x)[i])) ppush(new ParserTok((*x)[i])); else ppush(new ParserRule((*x)[i]));
                        free(tos);         
                    }
                }

                else {
                    if (tos->name() == cur) {
                        tos = ppop();
                        if (cur != Tokens::ENDFILE) {
                            if (tos->type() == "empty")
                                apush(new ast_empty());
                            else apush(new lit(cur, sc->getlexeme()));
                            if (flags.PARSER_TRACE)
                                aprint();
                        }
                        cur = sc->lex();
                        if (flags.SCANNER_TRACE)
                            cout << "next symbol: " 
                                 << tokname(cur) 
                                 << std::endl;
                        free(tos);
                    } else parse_unexpected_terminal_err((Tokens)tos->name(), (Tokens)cur);
                }
            }
            
            root = (ast_rt*)res_stack[0];
            return EXIT_SUCCESS;
        }


    private:
        Scanner *sc;
        ParseDict dict;
        vector<ast*> res_stack;
        vector<ParserItem*> pred_stack;
        ast_rt *root = nullptr;

};

#endif