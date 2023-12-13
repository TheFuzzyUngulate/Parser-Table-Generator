#ifndef P_OUTPUT_PARSER_H
#define P_OUTPUT_PARSER_H
#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define nullptr ((void*)0)

#define RULE_COUNT 6
#define STATE_COUNT 10
#define ELEMENT_COUNT 9
typedef void (*p_callback)(struct P_PARSE_CLASS* lst);

typedef struct P_PARSE_CLASS {
	P_ELEMENTS type;
	const char* name;
	P_PARSE_CLASS **children;
	int child_count;
} P_PARSE_CLASS;

p_callback funclist[RULE_COUNT];
typedef enum P_PARSER_ACTIONS {
	ERROR,
	SHIFT,
	GOTO,
	REDUCE,
	ACCEPT,
	CONFLICT
} P_PARSER_ACTIONS;
typedef struct P_TABLE_DATA {
	int name;
	int state;
	int funcindex;
	int size;
} P_TABLE_DATA;
P_TABLE_DATA **table;

void INIT_TABLE();
P_TABLE_DATA ACTION(int state, P_ELEMENTS tok);

#endif