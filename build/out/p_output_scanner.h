#ifndef P_OUTPUT_SCANNER_H
#define P_OUTPUT_SCANNER_H
#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define nullptr ((void*)0)

typedef enum P_ELEMENTS {
	P_START_LIT,
	P_TOK_END,
	P_LIT_START,
	P_LIT_E,
	P_LIT_T,
	P_LIT_E_BAR,
	P_TOK_1,
	P_TOK_2,
	P_LIT_F,
	P_LIT_T_BAR,
	P_TOK_3,
	P_TOK_4,
	P_TOK_5,
	P_TOK_6,
	P_TOK_NUM
} P_ELEMENTS;

typedef struct P_ELEMENT_LIST {
	P_ELEMENTS value;
	P_ELEMENT_LIST *next;
} P_ELEMENT_LIST;

typedef struct P_CHAR_LIST {
	char value;
	P_CHAR_LIST *next;
} P_CHAR_LIST;

typedef struct P_LEXER {
	int lineno;
	FILE* input_fd;
	const char *lexeme;
	P_CHAR_LIST *ungetch_list;
	P_ELEMENT_LIST *unlex_list;
} P_LEXER;

char GETCH();
void UNGETCH(char ch);
void UNLEX(P_ELEMENTS tok);
P_ELEMENTS LEX();
void LEX_INIT(const char *input);
P_LEXER *GET_LEX_DATA();

void P_LEX_ERROR(const char* message);

#endif