#include "p_output_scanner.h"
#include "p_output_parser.h"
void INIT_TABLE() {
	table = malloc(STATE_COUNT * sizeof(P_TABLE_DATA*));
	for (int i = 0; i < STATE_COUNT; ++i) {
		table[i] = malloc(ELEMENT_COUNT * sizeof(P_TABLE_DATA));
		memset(table[i], 0, sizeof(P_TABLE_DATA));
	}

	table[0][P_TOK_A].name = SHIFT;
	table[0][P_TOK_A].state = 1;
	table[0][P_TOK_A].funcindex = -1;
	table[0][P_TOK_A].size = 0;

	table[0][P_TOK_B].name = SHIFT;
	table[0][P_TOK_B].state = 2;
	table[0][P_TOK_B].funcindex = -1;
	table[0][P_TOK_B].size = 0;

	table[0][P_LIT_E].name = GOTO;
	table[0][P_LIT_E].state = 3;
	table[0][P_LIT_E].funcindex = -1;
	table[0][P_LIT_E].size = 0;

	table[0][P_LIT_START].name = GOTO;
	table[0][P_LIT_START].state = 4;
	table[0][P_LIT_START].funcindex = -1;
	table[0][P_LIT_START].size = 0;

	table[0][P_LIT_T].name = GOTO;
	table[0][P_LIT_T].state = 5;
	table[0][P_LIT_T].funcindex = -1;
	table[0][P_LIT_T].size = 0;

	table[5][P_TOK_1].name = SHIFT;
	table[5][P_TOK_1].state = 6;
	table[5][P_TOK_1].funcindex = -1;
	table[5][P_TOK_1].size = 0;

	table[5][P_LIT_T1].name = GOTO;
	table[5][P_LIT_T1].state = 7;
	table[5][P_LIT_T1].funcindex = -1;
	table[5][P_LIT_T1].size = 0;

	table[6][P_TOK_A].name = SHIFT;
	table[6][P_TOK_A].state = 1;
	table[6][P_TOK_A].funcindex = -1;
	table[6][P_TOK_A].size = 0;

	table[6][P_TOK_B].name = SHIFT;
	table[6][P_TOK_B].state = 2;
	table[6][P_TOK_B].funcindex = -1;
	table[6][P_TOK_B].size = 0;

	table[6][P_LIT_T].name = GOTO;
	table[6][P_LIT_T].state = 8;
	table[6][P_LIT_T].funcindex = -1;
	table[6][P_LIT_T].size = 0;

	table[8][P_TOK_1].name = SHIFT;
	table[8][P_TOK_1].state = 6;
	table[8][P_TOK_1].funcindex = -1;
	table[8][P_TOK_1].size = 0;

	table[8][P_LIT_T1].name = GOTO;
	table[8][P_LIT_T1].state = 9;
	table[8][P_LIT_T1].funcindex = -1;
	table[8][P_LIT_T1].size = 0;

	table[1][P_TOK_1].name = REDUCE;
	table[1][P_TOK_1].state = 0;
	table[1][P_TOK_1].funcindex = 0;
	table[1][P_TOK_1].size = 1;

	table[2][P_TOK_1].name = REDUCE;
	table[2][P_TOK_1].state = 0;
	table[2][P_TOK_1].funcindex = 0;
	table[2][P_TOK_1].size = 1;

	table[4][P_TOK_END].name = ACCEPT;
	table[4][P_TOK_END].state = 0;
	table[4][P_TOK_END].funcindex = -1;
	table[4][P_TOK_END].size = 0;

}

P_TABLE_DATA ACTION (int state, P_ELEMENTS tok) {
	return table[state][tok];
}

