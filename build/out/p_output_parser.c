#include "p_output_scanner.h"
#include "p_output_parser.h"
void INIT_TABLE() {
	table = malloc(P_STATE_COUNT * sizeof(P_TABLE_DATA*));
	for (int i = 0; i < P_STATE_COUNT; ++i) {
		table[i] = malloc(P_ELEMENT_COUNT * sizeof(P_TABLE_DATA));
		memset(table[i], 0, sizeof(P_TABLE_DATA));
	}

	table[0][P_TOK_5].name = SHIFT;
	table[0][P_TOK_5].state = 1;
	table[0][P_TOK_5].funcindex = -1;
	table[0][P_TOK_5].size = 0;

	table[0][P_TOK_NUM].name = SHIFT;
	table[0][P_TOK_NUM].state = 2;
	table[0][P_TOK_NUM].funcindex = -1;
	table[0][P_TOK_NUM].size = 0;

	table[0][P_LIT_E].name = GOTO;
	table[0][P_LIT_E].state = 3;
	table[0][P_LIT_E].funcindex = -1;
	table[0][P_LIT_E].size = 0;

	table[0][P_LIT_F].name = GOTO;
	table[0][P_LIT_F].state = 4;
	table[0][P_LIT_F].funcindex = -1;
	table[0][P_LIT_F].size = 0;

	table[0][P_LIT_START].name = GOTO;
	table[0][P_LIT_START].state = 5;
	table[0][P_LIT_START].funcindex = -1;
	table[0][P_LIT_START].size = 0;

	table[0][P_LIT_T].name = GOTO;
	table[0][P_LIT_T].state = 6;
	table[0][P_LIT_T].funcindex = -1;
	table[0][P_LIT_T].size = 0;

	table[1][P_TOK_5].name = SHIFT;
	table[1][P_TOK_5].state = 1;
	table[1][P_TOK_5].funcindex = -1;
	table[1][P_TOK_5].size = 0;

	table[1][P_TOK_NUM].name = SHIFT;
	table[1][P_TOK_NUM].state = 2;
	table[1][P_TOK_NUM].funcindex = -1;
	table[1][P_TOK_NUM].size = 0;

	table[1][P_LIT_E].name = GOTO;
	table[1][P_LIT_E].state = 7;
	table[1][P_LIT_E].funcindex = -1;
	table[1][P_LIT_E].size = 0;

	table[1][P_LIT_F].name = GOTO;
	table[1][P_LIT_F].state = 4;
	table[1][P_LIT_F].funcindex = -1;
	table[1][P_LIT_F].size = 0;

	table[1][P_LIT_T].name = GOTO;
	table[1][P_LIT_T].state = 6;
	table[1][P_LIT_T].funcindex = -1;
	table[1][P_LIT_T].size = 0;

	table[3][P_TOK_5].name = SHIFT;
	table[3][P_TOK_5].state = 1;
	table[3][P_TOK_5].funcindex = -1;
	table[3][P_TOK_5].size = 0;

	table[3][P_TOK_NUM].name = SHIFT;
	table[3][P_TOK_NUM].state = 2;
	table[3][P_TOK_NUM].funcindex = -1;
	table[3][P_TOK_NUM].size = 0;

	table[3][P_LIT_E].name = GOTO;
	table[3][P_LIT_E].state = 3;
	table[3][P_LIT_E].funcindex = -1;
	table[3][P_LIT_E].size = 0;

	table[3][P_LIT_F].name = GOTO;
	table[3][P_LIT_F].state = 4;
	table[3][P_LIT_F].funcindex = -1;
	table[3][P_LIT_F].size = 0;

	table[3][P_LIT_START].name = GOTO;
	table[3][P_LIT_START].state = 8;
	table[3][P_LIT_START].funcindex = -1;
	table[3][P_LIT_START].size = 0;

	table[3][P_LIT_T].name = GOTO;
	table[3][P_LIT_T].state = 6;
	table[3][P_LIT_T].funcindex = -1;
	table[3][P_LIT_T].size = 0;

	table[4][P_TOK_5].name = SHIFT;
	table[4][P_TOK_5].state = 1;
	table[4][P_TOK_5].funcindex = -1;
	table[4][P_TOK_5].size = 0;

	table[4][P_TOK_1].name = SHIFT;
	table[4][P_TOK_1].state = 9;
	table[4][P_TOK_1].funcindex = -1;
	table[4][P_TOK_1].size = 0;

	table[4][P_TOK_NUM].name = SHIFT;
	table[4][P_TOK_NUM].state = 2;
	table[4][P_TOK_NUM].funcindex = -1;
	table[4][P_TOK_NUM].size = 0;

	table[4][P_LIT_F].name = GOTO;
	table[4][P_LIT_F].state = 10;
	table[4][P_LIT_F].funcindex = -1;
	table[4][P_LIT_F].size = 0;

	table[4][P_LIT_T_BAR].name = GOTO;
	table[4][P_LIT_T_BAR].state = 11;
	table[4][P_LIT_T_BAR].funcindex = -1;
	table[4][P_LIT_T_BAR].size = 0;

	table[6][P_TOK_5].name = SHIFT;
	table[6][P_TOK_5].state = 1;
	table[6][P_TOK_5].funcindex = -1;
	table[6][P_TOK_5].size = 0;

	table[6][P_TOK_NUM].name = SHIFT;
	table[6][P_TOK_NUM].state = 2;
	table[6][P_TOK_NUM].funcindex = -1;
	table[6][P_TOK_NUM].size = 0;

	table[6][P_LIT_E_BAR].name = GOTO;
	table[6][P_LIT_E_BAR].state = 12;
	table[6][P_LIT_E_BAR].funcindex = -1;
	table[6][P_LIT_E_BAR].size = 0;

	table[6][P_LIT_F].name = GOTO;
	table[6][P_LIT_F].state = 4;
	table[6][P_LIT_F].funcindex = -1;
	table[6][P_LIT_F].size = 0;

	table[6][P_LIT_T].name = GOTO;
	table[6][P_LIT_T].state = 13;
	table[6][P_LIT_T].funcindex = -1;
	table[6][P_LIT_T].size = 0;

	table[7][P_TOK_6].name = SHIFT;
	table[7][P_TOK_6].state = 14;
	table[7][P_TOK_6].funcindex = -1;
	table[7][P_TOK_6].size = 0;

	table[10][P_TOK_3].name = SHIFT;
	table[10][P_TOK_3].state = 15;
	table[10][P_TOK_3].funcindex = -1;
	table[10][P_TOK_3].size = 0;

	table[10][P_TOK_1].name = SHIFT;
	table[10][P_TOK_1].state = 9;
	table[10][P_TOK_1].funcindex = -1;
	table[10][P_TOK_1].size = 0;

	table[10][P_TOK_4].name = SHIFT;
	table[10][P_TOK_4].state = 16;
	table[10][P_TOK_4].funcindex = -1;
	table[10][P_TOK_4].size = 0;

	table[13][P_TOK_2].name = SHIFT;
	table[13][P_TOK_2].state = 17;
	table[13][P_TOK_2].funcindex = -1;
	table[13][P_TOK_2].size = 0;

	table[13][P_TOK_1].name = SHIFT;
	table[13][P_TOK_1].state = 18;
	table[13][P_TOK_1].funcindex = -1;
	table[13][P_TOK_1].size = 0;

	table[2][P_TOK_5].name = REDUCE;
	table[2][P_TOK_5].state = 0;
	table[2][P_TOK_5].funcindex = 0;
	table[2][P_TOK_5].size = 1;

	table[2][P_TOK_6].name = REDUCE;
	table[2][P_TOK_6].state = 0;
	table[2][P_TOK_6].funcindex = 0;
	table[2][P_TOK_6].size = 1;

	table[2][P_TOK_2].name = REDUCE;
	table[2][P_TOK_2].state = 0;
	table[2][P_TOK_2].funcindex = 0;
	table[2][P_TOK_2].size = 1;

	table[2][P_TOK_1].name = REDUCE;
	table[2][P_TOK_1].state = 0;
	table[2][P_TOK_1].funcindex = 0;
	table[2][P_TOK_1].size = 1;

	table[2][P_TOK_NUM].name = REDUCE;
	table[2][P_TOK_NUM].state = 0;
	table[2][P_TOK_NUM].funcindex = 0;
	table[2][P_TOK_NUM].size = 1;

	table[2][P_TOK_3].name = REDUCE;
	table[2][P_TOK_3].state = 0;
	table[2][P_TOK_3].funcindex = 0;
	table[2][P_TOK_3].size = 1;

	table[2][P_TOK_4].name = REDUCE;
	table[2][P_TOK_4].state = 0;
	table[2][P_TOK_4].funcindex = 0;
	table[2][P_TOK_4].size = 1;

	table[4][P_TOK_5].name = REDUCE;
	table[4][P_TOK_5].state = 11;
	table[4][P_TOK_5].funcindex = 4;
	table[4][P_TOK_5].size = 1;

	table[4][P_TOK_6].name = REDUCE;
	table[4][P_TOK_6].state = 11;
	table[4][P_TOK_6].funcindex = 4;
	table[4][P_TOK_6].size = 1;

	table[4][P_TOK_2].name = REDUCE;
	table[4][P_TOK_2].state = 11;
	table[4][P_TOK_2].funcindex = 4;
	table[4][P_TOK_2].size = 1;

	table[4][P_TOK_1].name = REDUCE;
	table[4][P_TOK_1].state = 11;
	table[4][P_TOK_1].funcindex = 4;
	table[4][P_TOK_1].size = 1;

	table[4][P_TOK_NUM].name = REDUCE;
	table[4][P_TOK_NUM].state = 11;
	table[4][P_TOK_NUM].funcindex = 4;
	table[4][P_TOK_NUM].size = 1;

	table[5][P_TOK_END].name = ACCEPT;
	table[5][P_TOK_END].state = 0;
	table[5][P_TOK_END].funcindex = -1;
	table[5][P_TOK_END].size = 0;

	table[6][P_TOK_5].name = REDUCE;
	table[6][P_TOK_5].state = 12;
	table[6][P_TOK_5].funcindex = 3;
	table[6][P_TOK_5].size = 1;

	table[6][P_TOK_6].name = REDUCE;
	table[6][P_TOK_6].state = 12;
	table[6][P_TOK_6].funcindex = 3;
	table[6][P_TOK_6].size = 1;

	table[6][P_TOK_NUM].name = REDUCE;
	table[6][P_TOK_NUM].state = 12;
	table[6][P_TOK_NUM].funcindex = 3;
	table[6][P_TOK_NUM].size = 1;

	table[9][P_TOK_5].name = REDUCE;
	table[9][P_TOK_5].state = 0;
	table[9][P_TOK_5].funcindex = 0;
	table[9][P_TOK_5].size = 2;

	table[9][P_TOK_6].name = REDUCE;
	table[9][P_TOK_6].state = 0;
	table[9][P_TOK_6].funcindex = 0;
	table[9][P_TOK_6].size = 2;

	table[9][P_TOK_2].name = REDUCE;
	table[9][P_TOK_2].state = 0;
	table[9][P_TOK_2].funcindex = 0;
	table[9][P_TOK_2].size = 2;

	table[9][P_TOK_1].name = REDUCE;
	table[9][P_TOK_1].state = 0;
	table[9][P_TOK_1].funcindex = 0;
	table[9][P_TOK_1].size = 2;

	table[9][P_TOK_NUM].name = REDUCE;
	table[9][P_TOK_NUM].state = 0;
	table[9][P_TOK_NUM].funcindex = 0;
	table[9][P_TOK_NUM].size = 2;

	table[9][P_TOK_3].name = REDUCE;
	table[9][P_TOK_3].state = 0;
	table[9][P_TOK_3].funcindex = 0;
	table[9][P_TOK_3].size = 2;

	table[9][P_TOK_4].name = REDUCE;
	table[9][P_TOK_4].state = 0;
	table[9][P_TOK_4].funcindex = 0;
	table[9][P_TOK_4].size = 2;

	table[11][P_TOK_5].name = REDUCE;
	table[11][P_TOK_5].state = 0;
	table[11][P_TOK_5].funcindex = 0;
	table[11][P_TOK_5].size = 2;

	table[11][P_TOK_6].name = REDUCE;
	table[11][P_TOK_6].state = 0;
	table[11][P_TOK_6].funcindex = 0;
	table[11][P_TOK_6].size = 2;

	table[11][P_TOK_NUM].name = REDUCE;
	table[11][P_TOK_NUM].state = 0;
	table[11][P_TOK_NUM].funcindex = 0;
	table[11][P_TOK_NUM].size = 2;

	table[11][P_TOK_2].name = REDUCE;
	table[11][P_TOK_2].state = 0;
	table[11][P_TOK_2].funcindex = 0;
	table[11][P_TOK_2].size = 2;

	table[11][P_TOK_1].name = REDUCE;
	table[11][P_TOK_1].state = 0;
	table[11][P_TOK_1].funcindex = 0;
	table[11][P_TOK_1].size = 2;

	table[12][P_TOK_5].name = REDUCE;
	table[12][P_TOK_5].state = 0;
	table[12][P_TOK_5].funcindex = 0;
	table[12][P_TOK_5].size = 2;

	table[12][P_TOK_NUM].name = REDUCE;
	table[12][P_TOK_NUM].state = 0;
	table[12][P_TOK_NUM].funcindex = 0;
	table[12][P_TOK_NUM].size = 2;

	table[12][P_TOK_6].name = REDUCE;
	table[12][P_TOK_6].state = 0;
	table[12][P_TOK_6].funcindex = 0;
	table[12][P_TOK_6].size = 2;

	table[14][P_TOK_5].name = REDUCE;
	table[14][P_TOK_5].state = 0;
	table[14][P_TOK_5].funcindex = 0;
	table[14][P_TOK_5].size = 3;

	table[14][P_TOK_6].name = REDUCE;
	table[14][P_TOK_6].state = 0;
	table[14][P_TOK_6].funcindex = 0;
	table[14][P_TOK_6].size = 3;

	table[14][P_TOK_2].name = REDUCE;
	table[14][P_TOK_2].state = 0;
	table[14][P_TOK_2].funcindex = 0;
	table[14][P_TOK_2].size = 3;

	table[14][P_TOK_1].name = REDUCE;
	table[14][P_TOK_1].state = 0;
	table[14][P_TOK_1].funcindex = 0;
	table[14][P_TOK_1].size = 3;

	table[14][P_TOK_NUM].name = REDUCE;
	table[14][P_TOK_NUM].state = 0;
	table[14][P_TOK_NUM].funcindex = 0;
	table[14][P_TOK_NUM].size = 3;

	table[14][P_TOK_3].name = REDUCE;
	table[14][P_TOK_3].state = 0;
	table[14][P_TOK_3].funcindex = 0;
	table[14][P_TOK_3].size = 3;

	table[14][P_TOK_4].name = REDUCE;
	table[14][P_TOK_4].state = 0;
	table[14][P_TOK_4].funcindex = 0;
	table[14][P_TOK_4].size = 3;

	table[15][P_TOK_5].name = REDUCE;
	table[15][P_TOK_5].state = 0;
	table[15][P_TOK_5].funcindex = 0;
	table[15][P_TOK_5].size = 2;

	table[15][P_TOK_6].name = REDUCE;
	table[15][P_TOK_6].state = 0;
	table[15][P_TOK_6].funcindex = 0;
	table[15][P_TOK_6].size = 2;

	table[15][P_TOK_2].name = REDUCE;
	table[15][P_TOK_2].state = 0;
	table[15][P_TOK_2].funcindex = 0;
	table[15][P_TOK_2].size = 2;

	table[15][P_TOK_1].name = REDUCE;
	table[15][P_TOK_1].state = 0;
	table[15][P_TOK_1].funcindex = 0;
	table[15][P_TOK_1].size = 2;

	table[15][P_TOK_NUM].name = REDUCE;
	table[15][P_TOK_NUM].state = 0;
	table[15][P_TOK_NUM].funcindex = 0;
	table[15][P_TOK_NUM].size = 2;

	table[16][P_TOK_5].name = REDUCE;
	table[16][P_TOK_5].state = 0;
	table[16][P_TOK_5].funcindex = 0;
	table[16][P_TOK_5].size = 2;

	table[16][P_TOK_6].name = REDUCE;
	table[16][P_TOK_6].state = 0;
	table[16][P_TOK_6].funcindex = 0;
	table[16][P_TOK_6].size = 2;

	table[16][P_TOK_2].name = REDUCE;
	table[16][P_TOK_2].state = 0;
	table[16][P_TOK_2].funcindex = 0;
	table[16][P_TOK_2].size = 2;

	table[16][P_TOK_1].name = REDUCE;
	table[16][P_TOK_1].state = 0;
	table[16][P_TOK_1].funcindex = 0;
	table[16][P_TOK_1].size = 2;

	table[16][P_TOK_NUM].name = REDUCE;
	table[16][P_TOK_NUM].state = 0;
	table[16][P_TOK_NUM].funcindex = 0;
	table[16][P_TOK_NUM].size = 2;

	table[17][P_TOK_5].name = REDUCE;
	table[17][P_TOK_5].state = 0;
	table[17][P_TOK_5].funcindex = 0;
	table[17][P_TOK_5].size = 2;

	table[17][P_TOK_6].name = REDUCE;
	table[17][P_TOK_6].state = 0;
	table[17][P_TOK_6].funcindex = 0;
	table[17][P_TOK_6].size = 2;

	table[17][P_TOK_NUM].name = REDUCE;
	table[17][P_TOK_NUM].state = 0;
	table[17][P_TOK_NUM].funcindex = 0;
	table[17][P_TOK_NUM].size = 2;

	table[18][P_TOK_5].name = REDUCE;
	table[18][P_TOK_5].state = 0;
	table[18][P_TOK_5].funcindex = 0;
	table[18][P_TOK_5].size = 2;

	table[18][P_TOK_6].name = REDUCE;
	table[18][P_TOK_6].state = 0;
	table[18][P_TOK_6].funcindex = 0;
	table[18][P_TOK_6].size = 2;

	table[18][P_TOK_NUM].name = REDUCE;
	table[18][P_TOK_NUM].state = 0;
	table[18][P_TOK_NUM].funcindex = 0;
	table[18][P_TOK_NUM].size = 2;

}

P_TABLE_DATA ACTION (int state, P_ELEMENTS tok) {
	return table[state][tok];
}

void PARSE () {
	P_LIST_TYPE states = P_LIST_INIT(sizeof(int));
	P_LIST_TYPE values = P_LIST_INIT(sizeof(P_PARSE_CLASS));

	P_ELEMENTS tok = LEX();
	int startstate = P_START_LIT;
	P_LIST_PUSH(&states, &startstate);

	while (1) {
		int *current = (int*)P_LIST_TOP(&states);
		P_TABLE_DATA act = ACTION(*current, tok);
	}
}

