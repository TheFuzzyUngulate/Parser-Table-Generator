#include "p_output_scanner.h"
#include "p_output_parser.h"
P_TABLE_DATA **INIT_TABLE() {
	P_TABLE_DATA **table = malloc(P_STATE_COUNT * sizeof(P_TABLE_DATA*));
	for (int i = 0; i < P_STATE_COUNT; ++i) {
		table[i] = malloc(P_ELEMENT_COUNT * sizeof(P_TABLE_DATA));
		memset(table[i], 0, sizeof(P_TABLE_DATA));
	}

	return table;
}

P_TABLE_DATA ACTION (P_TABLE_DATA **table, int state, P_ELEMENTS tok) {
	return table[state][tok];
}

void PARSE (const char *filename) {
	P_LEXER info = LEX_INIT(filename);
	P_LIST_TYPE states = P_LIST_INIT(sizeof(int));
	P_LIST_TYPE values = P_LIST_INIT(sizeof(P_PARSE_CLASS));
P_TABLE_DATA **table = INIT_TABLE();

	P_ELEMENTS tok = LEX(&info);
	int startstate = P_START_LIT;
	P_LIST_PUSH(&states, &startstate);

	while (1) {
		int *current = (int*)P_LIST_TOP(&states);
		P_TABLE_DATA act = ACTION(table, *current, tok);
	}
}

