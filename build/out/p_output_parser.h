#ifndef P_OUTPUT_PARSER_H
#define P_OUTPUT_PARSER_H
#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "p_list_type.h"
#include "p_output_scanner.h"

#define nullptr ((void*)0)

#define P_RULE_COUNT 15
#define P_STATE_COUNT 33
#define P_ELEMENT_COUNT 15
typedef struct P_PARSE_CLASS {
	P_ELEMENTS type;
	const char* name;
	struct P_PARSE_CLASS **children;
	int child_count;
} P_PARSE_CLASS;

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
typedef struct P_ARRAY {
	int length;
	unsigned int i_size;
	void *ptr;
} P_ARRAY;
P_TABLE_DATA** INIT_TABLE();
P_TABLE_DATA ACTION(P_TABLE_DATA **table, int state, P_ELEMENTS tok);
void PARSE(const char* filename);
#endif