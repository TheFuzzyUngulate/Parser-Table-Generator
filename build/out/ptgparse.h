#ifndef PTGPARSE_H
#define PTGPARSE_H
#pragma once

#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ptg_enum {
	P_START_LIT,
	P_TOK_END,
	P_LIT_start,
	P_LIT_item,
	P_LIT_start_BAR,
	P_LIT_struct,
	P_LIT_directive,
	P_TOK_ELEMENT,
	P_TOK_COLON,
	P_LIT_struct_BAR,
	P_LIT_attribute,
	P_TOK_ATTRIBUTE,
	P_LIT_litlist,
	P_TOK_ELLIPSIS,
	P_LIT_literal,
	P_LIT_feature,
	P_TOK_STRING,
	P_TOK_ADDRESS,
	P_LIT_feature_BAR,
	P_LIT_featlist,
	P_TOK_LBRACK,
	P_LIT_featlist_BAR,
	P_TOK_RBRACK,
	P_TOK_COMMA,
	P_TOK_DIRECTIVE,
	P_LIT_diritem,
	P_LIT_directive_BAR
} ptg_enum;

static inline char*
ptg_enum_string(ptg_enum tok) {
	switch(tok) {
		case P_START_LIT: return "S*";
		case P_TOK_END: return "$";
		case P_LIT_start: return "start";
		case P_LIT_item: return "item";
		case P_LIT_start_BAR: return "start'";
		case P_LIT_struct: return "struct";
		case P_LIT_directive: return "directive";
		case P_TOK_ELEMENT: return "#ELEMENT";
		case P_TOK_COLON: return "#COLON";
		case P_LIT_struct_BAR: return "struct'";
		case P_LIT_attribute: return "attribute";
		case P_TOK_ATTRIBUTE: return "#ATTRIBUTE";
		case P_LIT_litlist: return "litlist";
		case P_TOK_ELLIPSIS: return "#ELLIPSIS";
		case P_LIT_literal: return "literal";
		case P_LIT_feature: return "feature";
		case P_TOK_STRING: return "#STRING";
		case P_TOK_ADDRESS: return "#ADDRESS";
		case P_LIT_feature_BAR: return "feature'";
		case P_LIT_featlist: return "featlist";
		case P_TOK_LBRACK: return "#LBRACK";
		case P_LIT_featlist_BAR: return "featlist'";
		case P_TOK_RBRACK: return "#RBRACK";
		case P_TOK_COMMA: return "#COMMA";
		case P_TOK_DIRECTIVE: return "#DIRECTIVE";
		case P_LIT_diritem: return "diritem";
		case P_LIT_directive_BAR: return "directive'";
		default: return "?";
	}
}

static FILE* ptg_infile = NULL;

char* ptg_lexeme();
ptg_enum ptg_lex();
void ptg_unlex(ptg_enum tok);

#endif
