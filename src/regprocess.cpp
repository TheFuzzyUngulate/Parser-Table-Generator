#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <sstream>

#include "../include/regex/regprocess.hpp"

/* https://stackoverflow.com/questions/735126/are-there-alternate-implementations-of-gnu-getline-interface/47229318#47229318 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = (char*)malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = (char*)realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

char re_scan::getch() {
	char ch;
	if (unget.size() > 0) {
		ch = unget.back();
		unget.pop_back();
		lastchar = ch;
		return ch;
	}

	if (offset >= src.size()) {
		lastchar = -1;
		return -1;
	}

	ch = src[offset];
	offset++;
	lastchar = ch;
	return ch;
}

re_tk re_scan::lex()
{
	int ch;

	if (unlex.size() > 0) {
		auto r = unlex.back();
		unlex.pop_back();
		return r;
	}

	while (true)
	{
		ch = getch();
		switch (ch)
		{
			case '[': return P_TOK_LBRACK;
			case ']': return P_TOK_RBRACK;
			case '+': return P_TOK_PLUS;
			case '-': return P_TOK_MINUS;
			case '*': return P_TOK_TIMES;
			case '?': return P_TOK_QUESTION;
			case '|': return P_TOK_BAR;
			case '\\': return P_TOK_SLASH;
			case '^': return P_TOK_CAP;
			case '(': return P_TOK_LPAREN;
			case ')': return P_TOK_RPAREN;
			case 'n': return P_TOK_NEWLINE_CHAR;
			case 'r': return P_TOK_CRETURN_CHAR;
			case 't': return P_TOK_TABULATE_CHAR;
			case '.': return P_TOK_DOT;
			case -1:  return P_TOK_END;
			default:  return P_TOK_CHAR;
		}
	}
}

/**
 * so, the tokens allowed are
 * ']' '[' '?' '+' '*' '|' '\' '^' '-' '(' ')' CHAR
 * here, CHAR is just every other char one sees
 * also, escape characters are not handled in scanner level
 * 
 * <re>    ::= <exp> <re>
 *         | empty
 *         ;
 * 
 * <exp>   ::= <elem>
 *         | <elem> '?'
 *         | <elem> '+'
 *         | <elem> '*'
 *         | '[' <slct> ']'
 *         | '(' <re> ')'
 *         ;
 * 
 * <elem>  ::= CHAR
 *         | '^'
 *         | '-'
 *         | '\' <canc>
 *         ;
 * 
 * <canc>  ::= '?' 
 *         | '+' 
 *         | '\' 
 *         | '*' 
 *         | '[' 
 *         | ']' 
 *         | '(' 
 *         | ')' 
 *         | '|'
 *         ;
 * 
 * <slct>  ::= '^' <slin>
 *         | <slin>
 *         ;
 * 
 * <slin>  ::= CHAR <slinb>
 *         | CHAR '-' CHAR <slinb>
 *         ;
 * 
 * <slinb> ::= CHAR <slinb>
 *         | CHAR '-' CHAR <slinb>
 *         | empty
 *         ;
 */

#define P_RULE_COUNT 46
#define P_STATE_COUNT 59
#define P_ELEMENT_COUNT 31

#define GOTO   1
#define ERROR  0
#define SHIFT  2
#define REDUCE 3
#define ACCEPT 4

static inline char* 
re_pobj_print(re_pobj re)
{
	int len   = 0;
	char* buf = NULL;

	switch (re.action)
	{
		case GOTO:
			len    = snprintf(NULL, 0, "g%d", re.op.sgoto);
			buf    = (char*)malloc(len);
			buf[0] = 'g';
			itoa(re.op.sgoto, buf+1, 10);
			break;

		case SHIFT:
			len    = snprintf(NULL, 0, "s%d", re.op.shift);
			buf    = (char*)malloc(len);
			buf[0] = 's';
			itoa(re.op.shift, buf+1, 10);
			break;

		case REDUCE:
			len    = snprintf(NULL, 0, "r%d", re.op.reduce.rule);
			buf    = (char*)malloc(len);
			buf[0] = 'r';
			itoa(re.op.reduce.rule, buf+1, 10);
			break;

		case ERROR:
			buf = (char*)"error";
			break;

		case ACCEPT:
			buf = (char*)"accept";
			break;
	}

	return buf;
}


#define re_table_next(table, state, token) (*((table) + ((state) * P_ELEMENT_COUNT) + (token)))

static inline void
re_table_set(re_pobj* table, int state, re_tk token, re_pobj data)
{
    re_pobj old = re_table_next(table, state, token);
    if (old.action != ERROR) {
        if (old.action == SHIFT && data.action == REDUCE) {
            fprintf(stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            fprintf(stderr, "opting to shift...\n");
            return;
        }
        if (old.action == REDUCE && data.action == SHIFT) {
            fprintf(stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            fprintf(stderr, "opting to shift...\n");
        }
        if (old.action == REDUCE && data.action == REDUCE) {
            fprintf(stderr, "error: reduce-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            exit(EXIT_FAILURE);
        }
    }

    re_table_next(table, state, token) = data;
}

static inline re_pobj*
re_table_prepare()
{
    re_pobj* table = (re_pobj*)calloc(P_STATE_COUNT * P_ELEMENT_COUNT, sizeof(re_pobj));

	re_table_set(table, 0, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 1}});
	re_table_set(table, 0, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 0, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 0, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 4}});
	re_table_set(table, 0, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 5}});
	re_table_set(table, 0, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 6}});
	re_table_set(table, 0, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 7}});
	re_table_set(table, 0, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 0, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 0, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 0, P_LIT_elm, { .action = GOTO, .op = {.sgoto = 11}});
	re_table_set(table, 0, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 0, P_LIT_exp, { .action = GOTO, .op = {.sgoto = 13}});
	re_table_set(table, 0, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 14}});
	re_table_set(table, 0, P_LIT_msub, { .action = GOTO, .op = {.sgoto = 15}});
	re_table_set(table, 0, P_LIT_re, { .action = GOTO, .op = {.sgoto = 16}});
	re_table_set(table, 0, P_LIT_sub, { .action = GOTO, .op = {.sgoto = 17}});
	re_table_set(table, 5, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 18}});
	re_table_set(table, 5, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 5, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 5, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 19}});
	re_table_set(table, 5, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 5, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 5, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 5, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 5, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 20}});
	re_table_set(table, 5, P_LIT_slc, { .action = GOTO, .op = {.sgoto = 21}});
	re_table_set(table, 5, P_LIT_sli, { .action = GOTO, .op = {.sgoto = 22}});
	re_table_set(table, 6, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 1}});
	re_table_set(table, 6, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 6, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 6, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 4}});
	re_table_set(table, 6, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 5}});
	re_table_set(table, 6, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 6}});
	re_table_set(table, 6, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 7}});
	re_table_set(table, 6, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 6, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 6, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 6, P_LIT_elm, { .action = GOTO, .op = {.sgoto = 11}});
	re_table_set(table, 6, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 6, P_LIT_exp, { .action = GOTO, .op = {.sgoto = 13}});
	re_table_set(table, 6, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 14}});
	re_table_set(table, 6, P_LIT_msub, { .action = GOTO, .op = {.sgoto = 15}});
	re_table_set(table, 6, P_LIT_re, { .action = GOTO, .op = {.sgoto = 23}});
	re_table_set(table, 6, P_LIT_sub, { .action = GOTO, .op = {.sgoto = 17}});
	re_table_set(table, 9, P_TOK_BAR, { .action = SHIFT, .op = {.shift = 24}});
	re_table_set(table, 9, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 25}});
	re_table_set(table, 9, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 26}});
	re_table_set(table, 9, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 27}});
	re_table_set(table, 9, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 28}});
	re_table_set(table, 9, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 29}});
	re_table_set(table, 9, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 30}});
	re_table_set(table, 9, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 31}});
	re_table_set(table, 9, P_TOK_PLUS, { .action = SHIFT, .op = {.shift = 32}});
	re_table_set(table, 9, P_TOK_QUESTION, { .action = SHIFT, .op = {.shift = 33}});
	re_table_set(table, 9, P_TOK_RBRACK, { .action = SHIFT, .op = {.shift = 34}});
	re_table_set(table, 9, P_TOK_RPAREN, { .action = SHIFT, .op = {.shift = 35}});
	re_table_set(table, 9, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 36}});
	re_table_set(table, 9, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 37}});
	re_table_set(table, 9, P_TOK_TIMES, { .action = SHIFT, .op = {.shift = 38}});
	re_table_set(table, 13, P_TOK_BAR, { .action = SHIFT, .op = {.shift = 39}});
	re_table_set(table, 13, P_LIT_re_BAR, { .action = GOTO, .op = {.sgoto = 40}});
	re_table_set(table, 15, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 1}});
	re_table_set(table, 15, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 15, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 15, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 4}});
	re_table_set(table, 15, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 5}});
	re_table_set(table, 15, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 6}});
	re_table_set(table, 15, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 7}});
	re_table_set(table, 15, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 15, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 15, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 15, P_LIT_elm, { .action = GOTO, .op = {.sgoto = 11}});
	re_table_set(table, 15, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 15, P_LIT_exp_BAR, { .action = GOTO, .op = {.sgoto = 41}});
	re_table_set(table, 15, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 14}});
	re_table_set(table, 15, P_LIT_msub, { .action = GOTO, .op = {.sgoto = 42}});
	re_table_set(table, 15, P_LIT_sub, { .action = GOTO, .op = {.sgoto = 17}});
	re_table_set(table, 17, P_TOK_PLUS, { .action = SHIFT, .op = {.shift = 43}});
	re_table_set(table, 17, P_TOK_QUESTION, { .action = SHIFT, .op = {.shift = 44}});
	re_table_set(table, 17, P_TOK_TIMES, { .action = SHIFT, .op = {.shift = 45}});
	re_table_set(table, 17, P_LIT_msub_BAR, { .action = GOTO, .op = {.sgoto = 46}});
	re_table_set(table, 18, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 18, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 18, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 19}});
	re_table_set(table, 18, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 18, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 18, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 18, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 18, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 20}});
	re_table_set(table, 18, P_LIT_sli, { .action = GOTO, .op = {.sgoto = 47}});
	re_table_set(table, 20, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 48}});
	re_table_set(table, 21, P_TOK_RBRACK, { .action = SHIFT, .op = {.shift = 49}});
	re_table_set(table, 22, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 22, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 22, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 19}});
	re_table_set(table, 22, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 22, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 22, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 22, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 22, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 20}});
	re_table_set(table, 22, P_LIT_slc_BAR, { .action = GOTO, .op = {.sgoto = 50}});
	re_table_set(table, 22, P_LIT_sli, { .action = GOTO, .op = {.sgoto = 51}});
	re_table_set(table, 23, P_TOK_RPAREN, { .action = SHIFT, .op = {.shift = 52}});
	re_table_set(table, 39, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 1}});
	re_table_set(table, 39, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 39, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 39, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 4}});
	re_table_set(table, 39, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 5}});
	re_table_set(table, 39, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 6}});
	re_table_set(table, 39, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 7}});
	re_table_set(table, 39, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 39, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 39, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 39, P_LIT_elm, { .action = GOTO, .op = {.sgoto = 11}});
	re_table_set(table, 39, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 39, P_LIT_exp, { .action = GOTO, .op = {.sgoto = 53}});
	re_table_set(table, 39, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 14}});
	re_table_set(table, 39, P_LIT_msub, { .action = GOTO, .op = {.sgoto = 15}});
	re_table_set(table, 39, P_LIT_sub, { .action = GOTO, .op = {.sgoto = 17}});
	re_table_set(table, 42, P_TOK_CAP, { .action = SHIFT, .op = {.shift = 1}});
	re_table_set(table, 42, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 42, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 42, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 4}});
	re_table_set(table, 42, P_TOK_LBRACK, { .action = SHIFT, .op = {.shift = 5}});
	re_table_set(table, 42, P_TOK_LPAREN, { .action = SHIFT, .op = {.shift = 6}});
	re_table_set(table, 42, P_TOK_MINUS, { .action = SHIFT, .op = {.shift = 7}});
	re_table_set(table, 42, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 42, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 42, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 42, P_LIT_elm, { .action = GOTO, .op = {.sgoto = 11}});
	re_table_set(table, 42, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 42, P_LIT_exp_BAR, { .action = GOTO, .op = {.sgoto = 54}});
	re_table_set(table, 42, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 14}});
	re_table_set(table, 42, P_LIT_msub, { .action = GOTO, .op = {.sgoto = 42}});
	re_table_set(table, 42, P_LIT_sub, { .action = GOTO, .op = {.sgoto = 17}});
	re_table_set(table, 47, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 47, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 47, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 19}});
	re_table_set(table, 47, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 47, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 47, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 47, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 47, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 20}});
	re_table_set(table, 47, P_LIT_slc_BAR, { .action = GOTO, .op = {.sgoto = 55}});
	re_table_set(table, 47, P_LIT_sli, { .action = GOTO, .op = {.sgoto = 51}});
	re_table_set(table, 48, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 48, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 48, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 48, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 48, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 48, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 48, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 56}});
	re_table_set(table, 51, P_TOK_CHAR, { .action = SHIFT, .op = {.shift = 2}});
	re_table_set(table, 51, P_TOK_CRETURN_CHAR, { .action = SHIFT, .op = {.shift = 3}});
	re_table_set(table, 51, P_TOK_DOT, { .action = SHIFT, .op = {.shift = 19}});
	re_table_set(table, 51, P_TOK_NEWLINE_CHAR, { .action = SHIFT, .op = {.shift = 8}});
	re_table_set(table, 51, P_TOK_SLASH, { .action = SHIFT, .op = {.shift = 9}});
	re_table_set(table, 51, P_TOK_TABULATE_CHAR, { .action = SHIFT, .op = {.shift = 10}});
	re_table_set(table, 51, P_LIT_esc, { .action = GOTO, .op = {.sgoto = 12}});
	re_table_set(table, 51, P_LIT_fch, { .action = GOTO, .op = {.sgoto = 20}});
	re_table_set(table, 51, P_LIT_slc_BAR, { .action = GOTO, .op = {.sgoto = 57}});
	re_table_set(table, 51, P_LIT_sli, { .action = GOTO, .op = {.sgoto = 51}});
	re_table_set(table, 53, P_TOK_BAR, { .action = SHIFT, .op = {.shift = 39}});
	re_table_set(table, 53, P_LIT_re_BAR, { .action = GOTO, .op = {.sgoto = 58}});
	re_table_set(table, 0, P_TOK_END, {.action = REDUCE, .op = { .reduce = {3, 0, P_LIT_re}}});
	re_table_set(table, 0, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {3, 0, P_LIT_re}}});
	re_table_set(table, 1, P_TOK_END, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 1, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {16, 1, P_LIT_elm}}});
	re_table_set(table, 2, P_TOK_END, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 2, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {41, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_END, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 3, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {44, 1, P_LIT_fch}}});
	re_table_set(table, 4, P_TOK_END, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 4, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {18, 1, P_LIT_elm}}});
	re_table_set(table, 6, P_TOK_END, {.action = REDUCE, .op = { .reduce = {3, 0, P_LIT_re}}});
	re_table_set(table, 6, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {3, 0, P_LIT_re}}});
	re_table_set(table, 7, P_TOK_END, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 7, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {17, 1, P_LIT_elm}}});
	re_table_set(table, 8, P_TOK_END, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 8, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {43, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_END, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 10, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {42, 1, P_LIT_fch}}});
	re_table_set(table, 11, P_TOK_END, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 11, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {12, 1, P_LIT_sub}}});
	re_table_set(table, 12, P_TOK_END, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 12, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {45, 1, P_LIT_fch}}});
	re_table_set(table, 13, P_TOK_END, {.action = REDUCE, .op = { .reduce = {2, 0, P_LIT_re_BAR}}});
	re_table_set(table, 13, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {2, 0, P_LIT_re_BAR}}});
	re_table_set(table, 14, P_TOK_END, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 14, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {15, 1, P_LIT_elm}}});
	re_table_set(table, 15, P_TOK_END, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 15, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 15, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 16, P_TOK_END, {.action = ACCEPT, .op = {.accept = 0}});
	re_table_set(table, 17, P_TOK_END, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 17, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {11, 0, P_LIT_msub_BAR}}});
	re_table_set(table, 19, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 19, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {40, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 20, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {38, 1, P_LIT_sli}}});
	re_table_set(table, 22, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {36, 0, P_LIT_slc_BAR}}});
	re_table_set(table, 24, P_TOK_END, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 24, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {25, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_END, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 25, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {27, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_END, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 26, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {32, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_END, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 27, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {30, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_END, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 28, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {20, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_END, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 29, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {28, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_END, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 30, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {23, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_END, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 31, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {31, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_END, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 32, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {22, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_END, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 33, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {19, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_END, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 34, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {21, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_END, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 35, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {29, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_END, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 36, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {26, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_END, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 37, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {33, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_END, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 38, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {24, 2, P_LIT_esc}}});
	re_table_set(table, 40, P_TOK_END, {.action = REDUCE, .op = { .reduce = {0, 2, P_LIT_re}}});
	re_table_set(table, 40, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {0, 2, P_LIT_re}}});
	re_table_set(table, 41, P_TOK_END, {.action = REDUCE, .op = { .reduce = {4, 2, P_LIT_exp}}});
	re_table_set(table, 41, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {4, 2, P_LIT_exp}}});
	re_table_set(table, 41, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {4, 2, P_LIT_exp}}});
	re_table_set(table, 42, P_TOK_END, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 42, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 42, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {6, 0, P_LIT_exp_BAR}}});
	re_table_set(table, 43, P_TOK_END, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 43, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {9, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_END, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 44, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {8, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_END, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 45, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {10, 1, P_LIT_msub_BAR}}});
	re_table_set(table, 46, P_TOK_END, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 46, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {7, 2, P_LIT_msub}}});
	re_table_set(table, 47, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {36, 0, P_LIT_slc_BAR}}});
	re_table_set(table, 49, P_TOK_END, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 49, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {13, 3, P_LIT_sub}}});
	re_table_set(table, 50, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {37, 2, P_LIT_slc}}});
	re_table_set(table, 51, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {36, 0, P_LIT_slc_BAR}}});
	re_table_set(table, 52, P_TOK_END, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_CAP, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_LBRACK, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_LPAREN, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_MINUS, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_PLUS, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_QUESTION, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 52, P_TOK_TIMES, {.action = REDUCE, .op = { .reduce = {14, 3, P_LIT_sub}}});
	re_table_set(table, 53, P_TOK_END, {.action = REDUCE, .op = { .reduce = {2, 0, P_LIT_re_BAR}}});
	re_table_set(table, 53, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {2, 0, P_LIT_re_BAR}}});
	re_table_set(table, 54, P_TOK_END, {.action = REDUCE, .op = { .reduce = {5, 2, P_LIT_exp_BAR}}});
	re_table_set(table, 54, P_TOK_BAR, {.action = REDUCE, .op = { .reduce = {5, 2, P_LIT_exp_BAR}}});
	re_table_set(table, 54, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {5, 2, P_LIT_exp_BAR}}});
	re_table_set(table, 55, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {34, 3, P_LIT_slc}}});
	re_table_set(table, 56, P_TOK_CHAR, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_CRETURN_CHAR, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_DOT, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_NEWLINE_CHAR, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_SLASH, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 56, P_TOK_TABULATE_CHAR, {.action = REDUCE, .op = { .reduce = {39, 3, P_LIT_sli}}});
	re_table_set(table, 57, P_TOK_RBRACK, {.action = REDUCE, .op = { .reduce = {35, 2, P_LIT_slc_BAR}}});
	re_table_set(table, 58, P_TOK_END, {.action = REDUCE, .op = { .reduce = {1, 3, P_LIT_re_BAR}}});
	re_table_set(table, 58, P_TOK_RPAREN, {.action = REDUCE, .op = { .reduce = {1, 3, P_LIT_re_BAR}}});

	return table;
}

re_parse::re_parse(re_scan* sc)
{
	cid     = 0;
	scanner = sc;
	ststack = {};
	tkstack = {};
	restack = {};
	table   = re_table_prepare();
}

re_exp* re_parse::compute()
{
	int i;
	int tos;
	re_tk a;
	re_exp* retmp1;
	re_exp* retmp2;
	re_exp* retmp3;
	re_exp* retmp4;

	/* add to state stack a zero value */
	ststack.push_back(0);

	/* get TOS before algorithm begins */
	a = scanner->lex();

	while (1)
	{
		/* state stack should not be empty here */
		if (ststack.size() < 1) {
			fprintf(stderr, "empty state stack!");
			exit(EXIT_FAILURE);
		}

		/* set all the values for this round */
		tos  = ststack.back();
		next = re_table_next(table, tos, a);
		
		/* reset */
		retmp1 = NULL;
		retmp2 = NULL;
		retmp3 = NULL;
		retmp4 = NULL;

		switch (next.action)
		{
			case SHIFT:
				/* push next state to state stack */
				ststack.push_back(next.op.shift);
				
				/* create new regex S->END by char */
				retmp1 = re_exp_new((re_exp) {
					.tag = re_exp::char_exp,
					.op = {.charExp = (char)scanner->lastchar}
				});

				/* push this new regex to regex stack */
				restack.push_back(retmp1);

				/* get new token */
				a = scanner->lex();

				break;

			case REDUCE:
				/* handle different reductions */
				switch (next.op.reduce.rule) {
					case 2:
					case 3:
					case 6:
					case 11:
					case 36:
						/* empty statement */
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::empty_exp,
							.op = {.emptyExp = 0}
						});
						break;

					case 46:
					case 45:
					case 44:
					case 43:
					case 42:
					case 41:
						/* fch <- esc */
						/* fch <- CRETURN_CHAR */
						/* fch <- NEWLINE_CHAR */
						/* fch <- TABULATE_CHAR */
						/* fch <- CHAR_CHAR */
						retmp2 = restack.back(); // ...
						restack.pop_back();
						retmp1 = retmp2;
						break;
						
					case 40:
						/* sli <- DOT */
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::dot_exp,
							.op  = {.dotExp = 0}
						});
						break;

					case 39:
						/* sli <- fch '-' fch */
						retmp3 = restack.back(); // fch
						restack.pop_back();
						restack.pop_back();
						retmp2 = restack.back(); // fch
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::range_exp,
							.op  = { 
								.rangeExp = {
									.min = retmp2->op.charExp,
									.max = retmp3->op.charExp
								}
							}
						});
						break;

					case 38:
						/* sli <- fch */
						retmp2 = restack.back(); // esc, fch
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 37:
					case 35:
						/* slc' <- sli slc' */
						/* slc  <- sli slc' */
						retmp3 = restack.back(); // slc'
						restack.pop_back();
						retmp2 = restack.back(); // sli
						restack.pop_back();

						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag          = re_exp::select_exp,
								.op = { .selectExp = {
									.pos    = 1,
									.select = re_comp_new((re_comp) {
										.elem = retmp2,
										.next = NULL
									})
								}}
							});
						}
						else
						if (retmp3->tag == re_exp::select_exp) {
							retmp3->op.selectExp.select = re_comp_new((re_comp) {
								.elem = retmp2,
								.next = retmp3->op.selectExp.select
							});
							retmp1 = retmp3;
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 34:
						/* slc <- CAP sli slc' */
						retmp3 = restack.back(); // slc'
						restack.pop_back();
						retmp2 = restack.back(); // sli
						restack.pop_back();
						restack.pop_back();

						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag          = re_exp::select_exp,
								.op = { .selectExp = {
									.pos    = 0,
									.select = re_comp_new((re_comp) {
										.elem = retmp2,
										.next = NULL
									})
								}}
							});
						}
						else
						if (retmp3->tag == re_exp::select_exp) {
							retmp3->op.selectExp.pos    = 0;
							retmp3->op.selectExp.select = re_comp_new((re_comp) {
								.elem = retmp2,
								.next = retmp3->op.selectExp.select
							});
							retmp1 = retmp3;
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 33:
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::char_exp,
							.op = {.charExp = '\t'}
						});
						break;

					case 32:
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::char_exp,
							.op = {.charExp = '\r'}
						});
						break;
						
					case 31:
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::char_exp,
							.op = {.charExp = '\n'}
						});
						break;
					
					case 30:
					case 29:
					case 28:
					case 27:
					case 26:
					case 25:
					case 24:
					case 23:
					case 22:
					case 21:
					case 20:
					case 19:
						retmp2 = restack.back();
						restack.pop_back();
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 18:
						restack.pop_back();
						retmp1 = re_exp_new((re_exp) {
							.tag = re_exp::dot_exp,
							.op = {.dotExp = 0}
						});
						break;

					case 17:
					case 16:
					case 15:
						retmp2 = restack.back();
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 14:
					case 13:
						/* sub <- LBRACK slc RBRACK */
						/* sub <- LPAREN re RPAREN  */
						restack.pop_back();
						retmp2 = restack.back(); // slc, re
						restack.pop_back();
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 12:
						/* sub <- elm */
						retmp2 = restack.back();
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 10:
					case 9:
					case 8:
						/* msub' <- PLUS */
						/* msub' <- TIMES */
						/* msub' <- QUESTION */
						retmp2 = restack.back();
						restack.pop_back();
						retmp1 = retmp2;
						break;

					case 7:
						/* msub <- sub msub' */
						retmp3 = restack.back(); // msub'
						restack.pop_back();
						retmp2 = restack.back(); // sub
						restack.pop_back();

						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = retmp2;
						}
						else {
							if (retmp3->tag == re_exp::char_exp) {
								switch (retmp3->op.charExp) {
									case '*':
										retmp1 = re_exp_new((re_exp) {
											.tag          = re_exp::kleene_exp,
											.op = { .kleeneExp = re_comp_new({
												.elem = retmp2,
												.next = NULL
											})}
										});
										break;

									case '+':
										retmp1 = re_exp_new((re_exp) {
											.tag       = re_exp::rep_exp,
											.op = {.repExp = re_comp_new({
												.elem = retmp2,
												.next = NULL
											})}
										});
										break;
										
									case '?':
										retmp1 = re_exp_new((re_exp) {
											.tag       = re_exp::opt_exp,
											.op = {.optExp = re_comp_new({
												.elem = retmp2,
												.next = NULL
											})}
										});
										break;
									
									default:
										fprintf(stderr, "incorrect type\n");
										exit(EXIT_FAILURE);
								}
							}
							else {
								fprintf(stderr, "incorrect type\n");
								exit(EXIT_FAILURE);
							}
						}
						break;

					case 5:
					case 4:
						/* exp' <- msub exp' */
						/* exp  <- msub exp' */
						retmp3 = restack.back(); // exp'
						restack.pop_back();
						retmp2 = restack.back(); // sub
						restack.pop_back();
						
						if (retmp3->tag == re_exp::plain_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag         = re_exp::plain_exp,
								.op = {.plainExp = re_comp_new({
									.elem = retmp2,
									.next = retmp3->op.plainExp
								})}
							});
						}
						else
						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag         = re_exp::plain_exp,
								.op = {.plainExp = re_comp_new({
									.elem = retmp2,
									.next = NULL
								})}
							});
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 1:
						/* re' <- BAR exp re' */
						retmp3 = restack.back(); // re'
						restack.pop_back();
						retmp2 = restack.back(); // exp
						restack.pop_back();
						restack.pop_back();

						if (retmp3->tag == re_exp::bar_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag       = re_exp::bar_exp,
								.op = {.barExp = {
									.left  = NULL,
									.right = re_comp_new((re_comp) {
										.elem = re_exp_new((re_exp) {
											.tag = re_exp::bar_exp,
											.op = {.barExp = {
												.left  = retmp2->op.plainExp,
												.right = retmp3->op.barExp.right
											}}
										}),
										.next = NULL
									})
								}}
							});
						}
						else
						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag       = re_exp::bar_exp,
								.op = {.barExp = {
									.left  = NULL,
									.right = retmp2->op.plainExp
								}}
							});
						}

						break;

					case 0:
						/* re <- exp re' */
						retmp3 = restack.back(); // re'
						restack.pop_back();
						retmp2 = restack.back(); // exp
						restack.pop_back();

						if (retmp3->tag == re_exp::bar_exp) {
							retmp3->op.barExp.left = retmp2->op.plainExp;
							retmp1 = retmp3;
						}
						else
						if (retmp3->tag == re_exp::empty_exp) {
							retmp1 = retmp2;
						}

						break;

					default:
						fprintf(stderr, "state out of range.\n");
						exit(EXIT_FAILURE);
				}

				restack.push_back(retmp1);

				for (i = 0; i < next.op.reduce.count; ++i)
					ststack.pop_back();

				tos  = ststack.back();
				next = re_table_next(table, tos, next.op.reduce.lhs_tok);
				if (next.action == GOTO) {
					ststack.push_back(next.op.sgoto);
				} else {
					fprintf(stderr, "invalid token \"%s\" in state#%d.\n", re_tk_string(next.op.reduce.lhs_tok), tos);
					exit(EXIT_FAILURE);
				}

				//printf("\n");
				break;

			case GOTO:
				//printf("using a goto...\n\n");
				a = scanner->lex();
				ststack.push_back(next.op.sgoto);
				break;

			case ACCEPT:
				//printf("we out...\n\n");
				retmp1 = restack.back();
				restack.pop_back();
				return retmp1;

			case ERROR:
				fprintf(stderr, "invalid token \"%s\" in state#%d.\n", re_tk_string(a), tos);
				exit(EXIT_FAILURE);
		}
	}
}

#define PAD_COUNT 4
#define re_write(f, str, space) do {\
	*(f) << string((space), '\t') << (str);\
} while (0);

#define ch_to_str(ch) ((ch) == '\n' ? "\\n" : ((ch) == '\t' ? "\\t" : ((ch) == '\r' ? "\\r" : ((ch) == '\\' ? "\\\\" : ((ch) == '\"' ? "\\\"" : ((ch) == '\'' ? "\\\'" : string(1, (ch))))))))

void re_conv_rec(re_exp* re, stringstream *fptr, int space)
{
	int k         = 0;
	int curspace  = 0;
	int depth     = 0;
	re_exp* curr  = NULL;
	bool pol      = false;
	re_comp* iter = NULL;

	switch (re->tag)
	{
		case re_exp::char_exp:
			re_write(fptr, "save_bool(scan() == \'", space);
			re_write(fptr, ch_to_str(re->op.charExp), 0);
			re_write(fptr, "\');\n", 0);
			break;

		case re_exp::dot_exp:
			re_write(fptr, "save_bool(scan() != EOF);\n", space);
			break;

		case re_exp::range_exp:
			re_write(fptr, "ch = scan();\n", space);
			re_write(fptr, "save_bool(ch >= \'", space);
			re_write(fptr, ch_to_str(re->op.rangeExp.min), 0);
			re_write(fptr, "\' && ch <= \'", 0);
			re_write(fptr, ch_to_str(re->op.rangeExp.max), 0);
			re_write(fptr, "\');\n", 0);
			break;

		case re_exp::empty_exp:
			re_write(fptr, "save_bool(true);\n", space);
			break;

		case re_exp::kleene_exp:
		case re_exp::rep_exp:
			re_write(fptr, "save_pos();\n", space);
			re_write(fptr, "new_counter();\n", space);
			re_write(fptr, "while (true) {\n", space);
			
			re_conv_rec(re_exp_new({
				.tag = re_exp::plain_exp,
				.op = {.plainExp = re->tag == re_exp::kleene_exp ? re->op.kleeneExp : re->op.repExp}
			}), fptr, space + 1);

			re_write(fptr, "if (!load_bool()) {\n", space + 1);
			re_write(fptr, "load_pos();\n", space + 2);
			re_write(fptr, "break;\n", space + 1);
			re_write(fptr, "} else {\n", space + 2);
			re_write(fptr, "save_pos();\n", space + 2);
			re_write(fptr, "inc_counter();\n", space + 2);
			re_write(fptr, "}\n", space + 1);

			re_write(fptr, "} save_bool(count() > ", space);
			re_write(fptr, re->tag == re_exp::kleene_exp ? "-1" : "0", 0);
			re_write(fptr, ");\n", 0);

			break;

		case re_exp::opt_exp:
			re_write(fptr, "save_pos();\n", space);
			
			re_conv_rec(re_exp_new({
				.tag = re_exp::plain_exp,
				.op = {.plainExp = re->tag == re_exp::kleene_exp ? re->op.kleeneExp : re->op.repExp}
			}), fptr, space);

			re_write(fptr, "if (!load_bool())\n", space);
			re_write(fptr, "load_pos();\n", space + 1);
			re_write(fptr, "save_bool(true);\n", space);
			
			break;

		case re_exp::select_exp:
			pol  = re->op.selectExp.pos;
			iter = re->op.selectExp.select;

			if (iter) {
				if (iter->next) {
					re_write(fptr, "do {\n", space);		
					while (iter) {
						re_conv_rec(iter->elem, fptr, space + 1);
						re_write(fptr, "if (", space + 1);
						re_write(fptr, pol ? "" : "!", 0);
						re_write(fptr, "load_bool()) {\n", 0);
						re_write(fptr, "save_bool(true);\n", space + 2);
						re_write(fptr, "break;\n", space + 2);
						re_write(fptr, "}\n", space + 1);
						iter = iter->next;
					}
					re_write(fptr, "save_bool(false);\n", space + 1);
					re_write(fptr, "} while (0);\n", space);
				} else re_conv_rec(iter->elem, fptr, space);
			}

			break;

		case re_exp::bar_exp:
			if (re->op.barExp.left && re->op.barExp.right)
			{
				iter = re->op.barExp.left;
				re_write(fptr, "save_pos();\n", space);
				re_conv_rec(re_exp_new({
					.tag = re_exp::plain_exp,
					.op = {.plainExp = iter}
				}), fptr, space);
				re_write(fptr, "if (!load_bool()) {\n", space);
				iter = re->op.barExp.right;
				re_write(fptr, "load_pos();\n", space + 1);
				re_conv_rec(re_exp_new({
					.tag = re_exp::plain_exp,
					.op = {.plainExp = iter}
				}), fptr, space + 1);
				re_write(fptr, "} else save_bool(true);\n", space);
			}
			else {
				iter = re->op.barExp.left ? re->op.barExp.left : re->op.barExp.right;
				re_conv_rec(re_exp_new({
					.tag = re_exp::plain_exp,
					.op = {.plainExp = iter}
				}), fptr, space);
			}
			break;

		case re_exp::plain_exp:

			iter = re->op.plainExp;

			if (iter) {
				if (iter->next) {
					re_write(fptr, "do {\n", space);		
					while (iter) {
						re_conv_rec(iter->elem, fptr, space + 1);
						re_write(fptr, "if (!load_bool()) {\n", space + 1);
						re_write(fptr, "save_bool(false);\n", space + 2);
						re_write(fptr, "break;\n", space + 2);
						re_write(fptr, "}\n", space + 1);
						iter = iter->next;
					}
					re_write(fptr, "save_bool(true);\n", space + 1);
					re_write(fptr, "} while (0);\n", space);
				} else re_conv_rec(iter->elem, fptr, space);
			}

			break;
	}	
}

/* get string form of regular expression */
string re_conv(string str, int space)
{
	stringstream ss;
	re_scan  sc = re_scan( str );
	re_parse pr = re_parse( &sc );
	re_exp*  re = pr.compute();
	re_conv_rec( re, &ss, space );

	return ss.str();
}

#undef re_write
#undef ch_to_str