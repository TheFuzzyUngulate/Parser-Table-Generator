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
	P_LIT_Decl,
	P_LIT_Expr,
	P_TOK_PLUS,
	P_LIT_Decl_BAR,
	P_TOK_MINUS,
	P_LIT_Fact,
	P_TOK_MUL,
	P_LIT_Expr_BAR,
	P_TOK_DIV,
	P_TOK_NUM,
	P_TOK_RIGHT,
	P_TOK_LEFT
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
		case P_LIT_Decl: return "Decl"; 
		case P_LIT_Expr: return "Expr"; 
		case P_TOK_PLUS: return "#PLUS"; 
		case P_LIT_Decl_BAR: return "Decl'"; 
		case P_TOK_MINUS: return "#MINUS"; 
		case P_LIT_Fact: return "Fact"; 
		case P_TOK_MUL: return "#MUL"; 
		case P_LIT_Expr_BAR: return "Expr'"; 
		case P_TOK_DIV: return "#DIV"; 
		case P_TOK_NUM: return "#NUM"; 
		case P_TOK_RIGHT: return "#RIGHT"; 
		case P_TOK_LEFT: return "#LEFT"; 
		default: P_LEX_ERROR("invalid token.");
	}
}

char GETCH(P_LEXER *info);
void UNGETCH(P_LEXER *info, char ch);
void UNLEX(P_LEXER *info, P_ELEMENTS tok);
P_ELEMENTS LEX(P_LEXER *info);
P_LEXER LEX_INIT(const char *input);

#endif