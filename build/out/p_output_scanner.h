#ifndef P_OUTPUT_SCANNER_H
#define P_OUTPUT_SCANNER_H
#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define nullptr ((void*)0)

static inline void P_LEX_ERROR(const char* message) {
	printf("p_lex_error: %s\n", message);
	exit(-1);
}

typedef enum P_ELEMENTS {
	P_START_LIT,
	P_TOK_END,
	P_LIT_START,
	P_LIT_E,
	P_LIT_T,
	P_TOK_MINUS,
	P_LIT_E_BAR,
	P_TOK_PLUS,
	P_LIT_F,
	P_TOK_MUL,
	P_LIT_T_BAR,
	P_TOK_DIV,
	P_TOK_LPAR,
	P_TOK_RPAR,
	P_TOK_NUM
} P_ELEMENTS;

typedef struct P_ELEMENT_LIST {
	P_ELEMENTS value;
	struct P_ELEMENT_LIST *next;
} P_ELEMENT_LIST;

typedef struct P_CHAR_LIST {
	char value;
	struct P_CHAR_LIST *next;
} P_CHAR_LIST;

typedef struct P_LEXER {
	int lineno;
	FILE* input_fd;
	char *lexeme;
	P_CHAR_LIST *ungetch_list;
	P_ELEMENT_LIST *unlex_list;
} P_LEXER;

static inline char * P_ELEMENT_STR(P_ELEMENTS tok) {
	switch (tok) {
		case P_START_LIT: return "S*"; 
		case P_TOK_END: return "$"; 
		case P_LIT_START: return "START"; 
		case P_LIT_E: return "E"; 
		case P_LIT_T: return "T"; 
		case P_TOK_MINUS: return "#MINUS"; 
		case P_LIT_E_BAR: return "E'"; 
		case P_TOK_PLUS: return "#PLUS"; 
		case P_LIT_F: return "F"; 
		case P_TOK_MUL: return "#MUL"; 
		case P_LIT_T_BAR: return "T'"; 
		case P_TOK_DIV: return "#DIV"; 
		case P_TOK_LPAR: return "#LPAR"; 
		case P_TOK_RPAR: return "#RPAR"; 
		case P_TOK_NUM: return "#NUM"; 
		default: P_LEX_ERROR("invalid token.");
	}
}

char GETCH(P_LEXER *info);
void UNGETCH(P_LEXER *info, char ch);
void UNLEX(P_LEXER *info, P_ELEMENTS tok);
P_ELEMENTS LEX(P_LEXER *info);
P_LEXER LEX_INIT(const char *input);

#endif