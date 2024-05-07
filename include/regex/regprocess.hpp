#ifndef REGPROCESS_HPP
#define REGPROCESS_HPP
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <string.h>

using   std::vector,
        std::string,
        std::ifstream,
        std::ostream,
		std::ofstream,
		std::stringstream;

struct re_exp;
struct re_scan;
struct re_comp;
struct re_state;
struct re_parse;

#define SPACING_COUNT 3

static inline void re_exp_print(struct re_exp*, int);
static inline void re_comp_print(struct re_comp*, int);

#define getspacing(compt) do {\
	for (int i = 0; i < (compt)*SPACING_COUNT; ++i)\
		putchar(' ');\
} while (0);

typedef enum re_tk {
	P_START_LIT,
	P_TOK_END,
	P_LIT_re,
	P_LIT_exp,
	P_LIT_re_BAR,
	P_TOK_BAR,
	P_LIT_msub,
	P_LIT_exp_BAR,
	P_LIT_sub,
	P_LIT_msub_BAR,
	P_TOK_QUESTION,
	P_TOK_PLUS,
	P_TOK_TIMES,
	P_LIT_elm,
	P_TOK_LBRACK,
	P_LIT_slc,
	P_TOK_RBRACK,
	P_TOK_LPAREN,
	P_TOK_RPAREN,
	P_LIT_fch,
	P_TOK_CAP,
	P_TOK_MINUS,
	P_LIT_esc,
	P_TOK_DOT,
	P_TOK_SLASH,
	P_TOK_NEWLINE_CHAR,
	P_TOK_CRETURN_CHAR,
	P_TOK_TABULATE_CHAR,
	P_LIT_sli,
	P_LIT_slc_BAR,
	P_TOK_CHAR
} re_tk;

static inline const char*
re_tk_string(re_tk tok) {
	switch (tok) {
		case P_START_LIT: return "S*";
		case P_TOK_END: return "$";
		case P_LIT_re: return "re";
		case P_LIT_exp: return "exp";
		case P_LIT_re_BAR: return "re'";
		case P_TOK_BAR: return "#BAR";
		case P_LIT_msub: return "msub";
		case P_LIT_exp_BAR: return "exp'";
		case P_LIT_sub: return "sub";
		case P_LIT_msub_BAR: return "msub'";
		case P_TOK_QUESTION: return "#QUESTION";
		case P_TOK_PLUS: return "#PLUS";
		case P_TOK_TIMES: return "#TIMES";
		case P_LIT_elm: return "elm";
		case P_TOK_LBRACK: return "#LBRACK";
		case P_LIT_slc: return "slc";
		case P_TOK_RBRACK: return "#RBRACK";
		case P_TOK_LPAREN: return "#LPAREN";
		case P_TOK_RPAREN: return "#RPAREN";
		case P_LIT_fch: return "fch";
		case P_TOK_CAP: return "#CAP";
		case P_TOK_MINUS: return "#MINUS";
		case P_LIT_esc: return "esc";
		case P_TOK_DOT: return "#DOT";
		case P_TOK_SLASH: return "#SLASH";
		case P_TOK_NEWLINE_CHAR: return "#NEWLINE_CHAR";
		case P_TOK_CRETURN_CHAR: return "#CRETURN_CHAR";
		case P_TOK_TABULATE_CHAR: return "#TABULATE_CHAR";
		case P_LIT_sli: return "sli";
		case P_LIT_slc_BAR: return "slc'";
		case P_TOK_CHAR: return "#CHAR";
		default: return "?";
	}
}

typedef struct re_exp {
    enum { char_exp, empty_exp,
		   dot_exp, rep_exp, bar_exp, 
		   plain_exp, opt_exp, range_exp, 
		   select_exp, kleene_exp } 		   tag;
    union { char                               charExp;
			char                               emptyExp;
			char						       dotExp;
            struct re_comp*                    repExp;
            struct re_comp*                    plainExp;
            struct { struct re_comp* left;
                     struct re_comp* right; }  barExp;
            struct re_comp*                    optExp;
			struct { char min; char max; }     rangeExp;
            struct { int pos;
			         struct re_comp* select; } selectExp;
            struct re_comp*                    kleeneExp; } op;
} re_exp;

typedef struct re_comp {
    re_exp*         elem;
    struct re_comp* next;
} re_comp;

static inline void 
re_exp_print(re_exp* re, int ind)
{
	if (re)
	{
		switch(re->tag)
		{
			case re_exp::kleene_exp:
				getspacing(ind);
				printf("rep-exp:\n");
				re_comp_print(re->op.kleeneExp, ind+1);
				break;

			case re_exp::select_exp:
				getspacing(ind);
				printf("select-exp: %s\n", re->op.selectExp.pos ? "" : " not");
				re_comp_print(re->op.selectExp.select, ind+1);
				break;

			case re_exp::range_exp:
				getspacing(ind);
				printf(
					"range: %c-%c\n", 
					re->op.rangeExp.min,
					re->op.rangeExp.max
				);
				break;

			case re_exp::opt_exp:
				getspacing(ind);
				printf("opt-exp:\n");
				re_comp_print(re->op.optExp, ind+1);
				break;

			case re_exp::bar_exp:
				getspacing(ind);
				printf("bar-exp:\n");

				getspacing(ind+1);
				printf("alt #1:\n");
				re_comp_print(re->op.barExp.left, ind+2);
				
				getspacing(ind+1);
				printf("alt #2:\n");
				re_comp_print(re->op.barExp.right, ind+2);
				break;

			case re_exp::plain_exp:
				getspacing(ind);
				printf("plain-exp:\n");
				re_comp_print(re->op.plainExp, ind+1);
				break;

			case re_exp::rep_exp:
				getspacing(ind);
				printf("rep-exp:\n");
				re_comp_print(re->op.repExp, ind+1);
				break;

			case re_exp::empty_exp:
				getspacing(ind);
				printf("empty\n");
				break;

			case re_exp::char_exp:
				getspacing(ind);
				printf("char: %c\n", re->op.charExp);
				break;
		}
	}
}

static inline void
re_comp_print(re_comp* comp, int indent)
{
	re_comp* top;

	if (comp) {
		top = comp;
		while (top) {
			re_exp_print(top->elem, indent);
			top = top->next;
		}
	}
}

static inline re_exp*
re_exp_new(re_exp re) {
	re_exp* ptr = (re_exp*)malloc(sizeof(re_exp));
	if (ptr) *ptr = re;
	return ptr;
}

static inline re_comp* 
re_comp_new(re_comp re) {
	re_comp* ptr = (re_comp*)malloc(sizeof(re_comp));
	if (ptr) *ptr = re;
	return ptr;
}

struct re_scan
{
    int line;
	int offset;
    string src;
    int column;
    int lastchar;
	string unget;
	vector<re_tk> unlex;

    re_scan(string str) {
        line     = 0;
        lastchar = 0;
        src      = str;
        offset   = 0;
        column   = 0;
        unget    = "";
        unlex    = {};
    }

    char getch();
    void ungetch(char ch) {
        unget.push_back(ch);
    }
    void unlextok(re_tk tok) {
        unlex.push_back(tok);
    }

    re_tk lex();
};

typedef struct 
re_pobj
{
	int action;
    union { 
        int shift;
        int sgoto;
        char accept;
        char error;
        struct { 
            int rule;
            int count;
            re_tk lhs_tok;
        } reduce;
    } op;
}
re_pobj;

typedef struct
re_parse
{
	int             cid;
	re_pobj         next;
    re_pobj*        table;
    re_scan*        scanner;
    vector<int>     ststack;
	vector<re_tk>   tkstack;
	vector<re_exp*> restack;

    re_parse(re_scan* sc);
    re_exp* compute();
}
re_parse;

string re_conv(string str, int space);

#endif