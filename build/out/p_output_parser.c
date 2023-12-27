#include "p_output_scanner.h"
#include "p_output_parser.h"
P_TABLE_DATA **INIT_TABLE() {
	P_TABLE_DATA **table = malloc(P_STATE_COUNT * sizeof(P_TABLE_DATA*));
	for (int i = 0; i < P_STATE_COUNT; ++i) {
		table[i] = malloc(P_ELEMENT_COUNT * sizeof(P_TABLE_DATA));
		memset(table[i], 0, sizeof(P_TABLE_DATA));
	}

	table[0][P_TOK_LPAR].name = SHIFT;
	table[0][P_TOK_LPAR].state = 1;
	table[0][P_TOK_LPAR].funcindex = -1;
	table[0][P_TOK_LPAR].size = 0;

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

	table[1][P_TOK_LPAR].name = SHIFT;
	table[1][P_TOK_LPAR].state = 1;
	table[1][P_TOK_LPAR].funcindex = -1;
	table[1][P_TOK_LPAR].size = 0;

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

	table[3][P_TOK_LPAR].name = SHIFT;
	table[3][P_TOK_LPAR].state = 1;
	table[3][P_TOK_LPAR].funcindex = -1;
	table[3][P_TOK_LPAR].size = 0;

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

	table[4][P_TOK_DIV].name = SHIFT;
	table[4][P_TOK_DIV].state = 9;
	table[4][P_TOK_DIV].funcindex = -1;
	table[4][P_TOK_DIV].size = 0;

	table[4][P_TOK_MINUS].name = SHIFT;
	table[4][P_TOK_MINUS].state = 10;
	table[4][P_TOK_MINUS].funcindex = -1;
	table[4][P_TOK_MINUS].size = 0;

	table[4][P_TOK_MUL].name = SHIFT;
	table[4][P_TOK_MUL].state = 11;
	table[4][P_TOK_MUL].funcindex = -1;
	table[4][P_TOK_MUL].size = 0;

	table[6][P_TOK_MINUS].name = SHIFT;
	table[6][P_TOK_MINUS].state = 12;
	table[6][P_TOK_MINUS].funcindex = -1;
	table[6][P_TOK_MINUS].size = 0;

	table[6][P_TOK_PLUS].name = SHIFT;
	table[6][P_TOK_PLUS].state = 13;
	table[6][P_TOK_PLUS].funcindex = -1;
	table[6][P_TOK_PLUS].size = 0;

	table[7][P_TOK_RPAR].name = SHIFT;
	table[7][P_TOK_RPAR].state = 14;
	table[7][P_TOK_RPAR].funcindex = -1;
	table[7][P_TOK_RPAR].size = 0;

	table[9][P_TOK_LPAR].name = SHIFT;
	table[9][P_TOK_LPAR].state = 1;
	table[9][P_TOK_LPAR].funcindex = -1;
	table[9][P_TOK_LPAR].size = 0;

	table[9][P_TOK_NUM].name = SHIFT;
	table[9][P_TOK_NUM].state = 2;
	table[9][P_TOK_NUM].funcindex = -1;
	table[9][P_TOK_NUM].size = 0;

	table[9][P_LIT_F].name = GOTO;
	table[9][P_LIT_F].state = 15;
	table[9][P_LIT_F].funcindex = -1;
	table[9][P_LIT_F].size = 0;

	table[11][P_TOK_LPAR].name = SHIFT;
	table[11][P_TOK_LPAR].state = 1;
	table[11][P_TOK_LPAR].funcindex = -1;
	table[11][P_TOK_LPAR].size = 0;

	table[11][P_TOK_NUM].name = SHIFT;
	table[11][P_TOK_NUM].state = 2;
	table[11][P_TOK_NUM].funcindex = -1;
	table[11][P_TOK_NUM].size = 0;

	table[11][P_LIT_F].name = GOTO;
	table[11][P_LIT_F].state = 16;
	table[11][P_LIT_F].funcindex = -1;
	table[11][P_LIT_F].size = 0;

	table[12][P_TOK_LPAR].name = SHIFT;
	table[12][P_TOK_LPAR].state = 1;
	table[12][P_TOK_LPAR].funcindex = -1;
	table[12][P_TOK_LPAR].size = 0;

	table[12][P_TOK_NUM].name = SHIFT;
	table[12][P_TOK_NUM].state = 2;
	table[12][P_TOK_NUM].funcindex = -1;
	table[12][P_TOK_NUM].size = 0;

	table[12][P_LIT_F].name = GOTO;
	table[12][P_LIT_F].state = 4;
	table[12][P_LIT_F].funcindex = -1;
	table[12][P_LIT_F].size = 0;

	table[12][P_LIT_T].name = GOTO;
	table[12][P_LIT_T].state = 17;
	table[12][P_LIT_T].funcindex = -1;
	table[12][P_LIT_T].size = 0;

	table[13][P_TOK_LPAR].name = SHIFT;
	table[13][P_TOK_LPAR].state = 1;
	table[13][P_TOK_LPAR].funcindex = -1;
	table[13][P_TOK_LPAR].size = 0;

	table[13][P_TOK_NUM].name = SHIFT;
	table[13][P_TOK_NUM].state = 2;
	table[13][P_TOK_NUM].funcindex = -1;
	table[13][P_TOK_NUM].size = 0;

	table[13][P_LIT_F].name = GOTO;
	table[13][P_LIT_F].state = 4;
	table[13][P_LIT_F].funcindex = -1;
	table[13][P_LIT_F].size = 0;

	table[13][P_LIT_T].name = GOTO;
	table[13][P_LIT_T].state = 18;
	table[13][P_LIT_T].funcindex = -1;
	table[13][P_LIT_T].size = 0;

	table[15][P_TOK_LPAR].name = SHIFT;
	table[15][P_TOK_LPAR].state = 1;
	table[15][P_TOK_LPAR].funcindex = -1;
	table[15][P_TOK_LPAR].size = 0;

	table[15][P_TOK_MINUS].name = SHIFT;
	table[15][P_TOK_MINUS].state = 10;
	table[15][P_TOK_MINUS].funcindex = -1;
	table[15][P_TOK_MINUS].size = 0;

	table[15][P_TOK_NUM].name = SHIFT;
	table[15][P_TOK_NUM].state = 2;
	table[15][P_TOK_NUM].funcindex = -1;
	table[15][P_TOK_NUM].size = 0;

	table[15][P_LIT_F].name = GOTO;
	table[15][P_LIT_F].state = 19;
	table[15][P_LIT_F].funcindex = -1;
	table[15][P_LIT_F].size = 0;

	table[15][P_LIT_T_BAR].name = GOTO;
	table[15][P_LIT_T_BAR].state = 20;
	table[15][P_LIT_T_BAR].funcindex = -1;
	table[15][P_LIT_T_BAR].size = 0;

	table[16][P_TOK_LPAR].name = SHIFT;
	table[16][P_TOK_LPAR].state = 1;
	table[16][P_TOK_LPAR].funcindex = -1;
	table[16][P_TOK_LPAR].size = 0;

	table[16][P_TOK_MINUS].name = SHIFT;
	table[16][P_TOK_MINUS].state = 10;
	table[16][P_TOK_MINUS].funcindex = -1;
	table[16][P_TOK_MINUS].size = 0;

	table[16][P_TOK_NUM].name = SHIFT;
	table[16][P_TOK_NUM].state = 2;
	table[16][P_TOK_NUM].funcindex = -1;
	table[16][P_TOK_NUM].size = 0;

	table[16][P_LIT_F].name = GOTO;
	table[16][P_LIT_F].state = 19;
	table[16][P_LIT_F].funcindex = -1;
	table[16][P_LIT_F].size = 0;

	table[16][P_LIT_T_BAR].name = GOTO;
	table[16][P_LIT_T_BAR].state = 21;
	table[16][P_LIT_T_BAR].funcindex = -1;
	table[16][P_LIT_T_BAR].size = 0;

	table[17][P_TOK_LPAR].name = SHIFT;
	table[17][P_TOK_LPAR].state = 1;
	table[17][P_TOK_LPAR].funcindex = -1;
	table[17][P_TOK_LPAR].size = 0;

	table[17][P_TOK_NUM].name = SHIFT;
	table[17][P_TOK_NUM].state = 2;
	table[17][P_TOK_NUM].funcindex = -1;
	table[17][P_TOK_NUM].size = 0;

	table[17][P_LIT_E_BAR].name = GOTO;
	table[17][P_LIT_E_BAR].state = 22;
	table[17][P_LIT_E_BAR].funcindex = -1;
	table[17][P_LIT_E_BAR].size = 0;

	table[17][P_LIT_F].name = GOTO;
	table[17][P_LIT_F].state = 4;
	table[17][P_LIT_F].funcindex = -1;
	table[17][P_LIT_F].size = 0;

	table[17][P_LIT_T].name = GOTO;
	table[17][P_LIT_T].state = 23;
	table[17][P_LIT_T].funcindex = -1;
	table[17][P_LIT_T].size = 0;

	table[18][P_TOK_LPAR].name = SHIFT;
	table[18][P_TOK_LPAR].state = 1;
	table[18][P_TOK_LPAR].funcindex = -1;
	table[18][P_TOK_LPAR].size = 0;

	table[18][P_TOK_NUM].name = SHIFT;
	table[18][P_TOK_NUM].state = 2;
	table[18][P_TOK_NUM].funcindex = -1;
	table[18][P_TOK_NUM].size = 0;

	table[18][P_LIT_E_BAR].name = GOTO;
	table[18][P_LIT_E_BAR].state = 24;
	table[18][P_LIT_E_BAR].funcindex = -1;
	table[18][P_LIT_E_BAR].size = 0;

	table[18][P_LIT_F].name = GOTO;
	table[18][P_LIT_F].state = 4;
	table[18][P_LIT_F].funcindex = -1;
	table[18][P_LIT_F].size = 0;

	table[18][P_LIT_T].name = GOTO;
	table[18][P_LIT_T].state = 23;
	table[18][P_LIT_T].funcindex = -1;
	table[18][P_LIT_T].size = 0;

	table[19][P_TOK_DIV].name = SHIFT;
	table[19][P_TOK_DIV].state = 25;
	table[19][P_TOK_DIV].funcindex = -1;
	table[19][P_TOK_DIV].size = 0;

	table[19][P_TOK_MINUS].name = SHIFT;
	table[19][P_TOK_MINUS].state = 10;
	table[19][P_TOK_MINUS].funcindex = -1;
	table[19][P_TOK_MINUS].size = 0;

	table[19][P_TOK_MUL].name = SHIFT;
	table[19][P_TOK_MUL].state = 26;
	table[19][P_TOK_MUL].funcindex = -1;
	table[19][P_TOK_MUL].size = 0;

	table[23][P_TOK_MINUS].name = SHIFT;
	table[23][P_TOK_MINUS].state = 27;
	table[23][P_TOK_MINUS].funcindex = -1;
	table[23][P_TOK_MINUS].size = 0;

	table[23][P_TOK_PLUS].name = SHIFT;
	table[23][P_TOK_PLUS].state = 28;
	table[23][P_TOK_PLUS].funcindex = -1;
	table[23][P_TOK_PLUS].size = 0;

	table[25][P_TOK_LPAR].name = SHIFT;
	table[25][P_TOK_LPAR].state = 1;
	table[25][P_TOK_LPAR].funcindex = -1;
	table[25][P_TOK_LPAR].size = 0;

	table[25][P_TOK_NUM].name = SHIFT;
	table[25][P_TOK_NUM].state = 2;
	table[25][P_TOK_NUM].funcindex = -1;
	table[25][P_TOK_NUM].size = 0;

	table[25][P_LIT_F].name = GOTO;
	table[25][P_LIT_F].state = 19;
	table[25][P_LIT_F].funcindex = -1;
	table[25][P_LIT_F].size = 0;

	table[25][P_LIT_T_BAR].name = GOTO;
	table[25][P_LIT_T_BAR].state = 29;
	table[25][P_LIT_T_BAR].funcindex = -1;
	table[25][P_LIT_T_BAR].size = 0;

	table[26][P_TOK_LPAR].name = SHIFT;
	table[26][P_TOK_LPAR].state = 1;
	table[26][P_TOK_LPAR].funcindex = -1;
	table[26][P_TOK_LPAR].size = 0;

	table[26][P_TOK_NUM].name = SHIFT;
	table[26][P_TOK_NUM].state = 2;
	table[26][P_TOK_NUM].funcindex = -1;
	table[26][P_TOK_NUM].size = 0;

	table[26][P_LIT_F].name = GOTO;
	table[26][P_LIT_F].state = 19;
	table[26][P_LIT_F].funcindex = -1;
	table[26][P_LIT_F].size = 0;

	table[26][P_LIT_T_BAR].name = GOTO;
	table[26][P_LIT_T_BAR].state = 30;
	table[26][P_LIT_T_BAR].funcindex = -1;
	table[26][P_LIT_T_BAR].size = 0;

	table[27][P_TOK_LPAR].name = SHIFT;
	table[27][P_TOK_LPAR].state = 1;
	table[27][P_TOK_LPAR].funcindex = -1;
	table[27][P_TOK_LPAR].size = 0;

	table[27][P_TOK_NUM].name = SHIFT;
	table[27][P_TOK_NUM].state = 2;
	table[27][P_TOK_NUM].funcindex = -1;
	table[27][P_TOK_NUM].size = 0;

	table[27][P_LIT_E_BAR].name = GOTO;
	table[27][P_LIT_E_BAR].state = 31;
	table[27][P_LIT_E_BAR].funcindex = -1;
	table[27][P_LIT_E_BAR].size = 0;

	table[27][P_LIT_F].name = GOTO;
	table[27][P_LIT_F].state = 4;
	table[27][P_LIT_F].funcindex = -1;
	table[27][P_LIT_F].size = 0;

	table[27][P_LIT_T].name = GOTO;
	table[27][P_LIT_T].state = 23;
	table[27][P_LIT_T].funcindex = -1;
	table[27][P_LIT_T].size = 0;

	table[28][P_TOK_LPAR].name = SHIFT;
	table[28][P_TOK_LPAR].state = 1;
	table[28][P_TOK_LPAR].funcindex = -1;
	table[28][P_TOK_LPAR].size = 0;

	table[28][P_TOK_NUM].name = SHIFT;
	table[28][P_TOK_NUM].state = 2;
	table[28][P_TOK_NUM].funcindex = -1;
	table[28][P_TOK_NUM].size = 0;

	table[28][P_LIT_E_BAR].name = GOTO;
	table[28][P_LIT_E_BAR].state = 32;
	table[28][P_LIT_E_BAR].funcindex = -1;
	table[28][P_LIT_E_BAR].size = 0;

	table[28][P_LIT_F].name = GOTO;
	table[28][P_LIT_F].state = 4;
	table[28][P_LIT_F].funcindex = -1;
	table[28][P_LIT_F].size = 0;

	table[28][P_LIT_T].name = GOTO;
	table[28][P_LIT_T].state = 23;
	table[28][P_LIT_T].funcindex = -1;
	table[28][P_LIT_T].size = 0;

	table[2][P_TOK_DIV].name = REDUCE;
	table[2][P_TOK_DIV].state = 0;
	table[2][P_TOK_DIV].funcindex = 0;
	table[2][P_TOK_DIV].size = 1;

	table[2][P_TOK_MINUS].name = REDUCE;
	table[2][P_TOK_MINUS].state = 0;
	table[2][P_TOK_MINUS].funcindex = 0;
	table[2][P_TOK_MINUS].size = 1;

	table[2][P_TOK_MUL].name = REDUCE;
	table[2][P_TOK_MUL].state = 0;
	table[2][P_TOK_MUL].funcindex = 0;
	table[2][P_TOK_MUL].size = 1;

	table[2][P_TOK_LPAR].name = REDUCE;
	table[2][P_TOK_LPAR].state = 0;
	table[2][P_TOK_LPAR].funcindex = 0;
	table[2][P_TOK_LPAR].size = 1;

	table[2][P_TOK_NUM].name = REDUCE;
	table[2][P_TOK_NUM].state = 0;
	table[2][P_TOK_NUM].funcindex = 0;
	table[2][P_TOK_NUM].size = 1;

	table[2][P_TOK_PLUS].name = REDUCE;
	table[2][P_TOK_PLUS].state = 0;
	table[2][P_TOK_PLUS].funcindex = 0;
	table[2][P_TOK_PLUS].size = 1;

	table[2][P_TOK_RPAR].name = REDUCE;
	table[2][P_TOK_RPAR].state = 0;
	table[2][P_TOK_RPAR].funcindex = 0;
	table[2][P_TOK_RPAR].size = 1;

	table[5][P_TOK_END].name = ACCEPT;
	table[5][P_TOK_END].state = 0;
	table[5][P_TOK_END].funcindex = -1;
	table[5][P_TOK_END].size = 0;

	table[10][P_TOK_DIV].name = REDUCE;
	table[10][P_TOK_DIV].state = 0;
	table[10][P_TOK_DIV].funcindex = 0;
	table[10][P_TOK_DIV].size = 2;

	table[10][P_TOK_MINUS].name = REDUCE;
	table[10][P_TOK_MINUS].state = 0;
	table[10][P_TOK_MINUS].funcindex = 0;
	table[10][P_TOK_MINUS].size = 2;

	table[10][P_TOK_MUL].name = REDUCE;
	table[10][P_TOK_MUL].state = 0;
	table[10][P_TOK_MUL].funcindex = 0;
	table[10][P_TOK_MUL].size = 2;

	table[10][P_TOK_LPAR].name = REDUCE;
	table[10][P_TOK_LPAR].state = 0;
	table[10][P_TOK_LPAR].funcindex = 0;
	table[10][P_TOK_LPAR].size = 2;

	table[10][P_TOK_NUM].name = REDUCE;
	table[10][P_TOK_NUM].state = 0;
	table[10][P_TOK_NUM].funcindex = 0;
	table[10][P_TOK_NUM].size = 2;

	table[10][P_TOK_PLUS].name = REDUCE;
	table[10][P_TOK_PLUS].state = 0;
	table[10][P_TOK_PLUS].funcindex = 0;
	table[10][P_TOK_PLUS].size = 2;

	table[10][P_TOK_RPAR].name = REDUCE;
	table[10][P_TOK_RPAR].state = 0;
	table[10][P_TOK_RPAR].funcindex = 0;
	table[10][P_TOK_RPAR].size = 2;

	table[14][P_TOK_DIV].name = REDUCE;
	table[14][P_TOK_DIV].state = 0;
	table[14][P_TOK_DIV].funcindex = 0;
	table[14][P_TOK_DIV].size = 3;

	table[14][P_TOK_MINUS].name = REDUCE;
	table[14][P_TOK_MINUS].state = 0;
	table[14][P_TOK_MINUS].funcindex = 0;
	table[14][P_TOK_MINUS].size = 3;

	table[14][P_TOK_MUL].name = REDUCE;
	table[14][P_TOK_MUL].state = 0;
	table[14][P_TOK_MUL].funcindex = 0;
	table[14][P_TOK_MUL].size = 3;

	table[14][P_TOK_LPAR].name = REDUCE;
	table[14][P_TOK_LPAR].state = 0;
	table[14][P_TOK_LPAR].funcindex = 0;
	table[14][P_TOK_LPAR].size = 3;

	table[14][P_TOK_NUM].name = REDUCE;
	table[14][P_TOK_NUM].state = 0;
	table[14][P_TOK_NUM].funcindex = 0;
	table[14][P_TOK_NUM].size = 3;

	table[14][P_TOK_PLUS].name = REDUCE;
	table[14][P_TOK_PLUS].state = 0;
	table[14][P_TOK_PLUS].funcindex = 0;
	table[14][P_TOK_PLUS].size = 3;

	table[14][P_TOK_RPAR].name = REDUCE;
	table[14][P_TOK_RPAR].state = 0;
	table[14][P_TOK_RPAR].funcindex = 0;
	table[14][P_TOK_RPAR].size = 3;

	table[15][P_TOK_LPAR].name = REDUCE;
	table[15][P_TOK_LPAR].state = 20;
	table[15][P_TOK_LPAR].funcindex = 4;
	table[15][P_TOK_LPAR].size = 1;

	table[15][P_TOK_MINUS].name = REDUCE;
	table[15][P_TOK_MINUS].state = 20;
	table[15][P_TOK_MINUS].funcindex = 4;
	table[15][P_TOK_MINUS].size = 1;

	table[15][P_TOK_NUM].name = REDUCE;
	table[15][P_TOK_NUM].state = 20;
	table[15][P_TOK_NUM].funcindex = 4;
	table[15][P_TOK_NUM].size = 1;

	table[15][P_TOK_PLUS].name = REDUCE;
	table[15][P_TOK_PLUS].state = 20;
	table[15][P_TOK_PLUS].funcindex = 4;
	table[15][P_TOK_PLUS].size = 1;

	table[15][P_TOK_RPAR].name = REDUCE;
	table[15][P_TOK_RPAR].state = 20;
	table[15][P_TOK_RPAR].funcindex = 4;
	table[15][P_TOK_RPAR].size = 1;

	table[16][P_TOK_LPAR].name = REDUCE;
	table[16][P_TOK_LPAR].state = 21;
	table[16][P_TOK_LPAR].funcindex = 4;
	table[16][P_TOK_LPAR].size = 1;

	table[16][P_TOK_MINUS].name = REDUCE;
	table[16][P_TOK_MINUS].state = 21;
	table[16][P_TOK_MINUS].funcindex = 4;
	table[16][P_TOK_MINUS].size = 1;

	table[16][P_TOK_NUM].name = REDUCE;
	table[16][P_TOK_NUM].state = 21;
	table[16][P_TOK_NUM].funcindex = 4;
	table[16][P_TOK_NUM].size = 1;

	table[16][P_TOK_PLUS].name = REDUCE;
	table[16][P_TOK_PLUS].state = 21;
	table[16][P_TOK_PLUS].funcindex = 4;
	table[16][P_TOK_PLUS].size = 1;

	table[16][P_TOK_RPAR].name = REDUCE;
	table[16][P_TOK_RPAR].state = 21;
	table[16][P_TOK_RPAR].funcindex = 4;
	table[16][P_TOK_RPAR].size = 1;

	table[17][P_TOK_LPAR].name = REDUCE;
	table[17][P_TOK_LPAR].state = 22;
	table[17][P_TOK_LPAR].funcindex = 3;
	table[17][P_TOK_LPAR].size = 1;

	table[17][P_TOK_NUM].name = REDUCE;
	table[17][P_TOK_NUM].state = 22;
	table[17][P_TOK_NUM].funcindex = 3;
	table[17][P_TOK_NUM].size = 1;

	table[17][P_TOK_RPAR].name = REDUCE;
	table[17][P_TOK_RPAR].state = 22;
	table[17][P_TOK_RPAR].funcindex = 3;
	table[17][P_TOK_RPAR].size = 1;

	table[18][P_TOK_LPAR].name = REDUCE;
	table[18][P_TOK_LPAR].state = 24;
	table[18][P_TOK_LPAR].funcindex = 3;
	table[18][P_TOK_LPAR].size = 1;

	table[18][P_TOK_NUM].name = REDUCE;
	table[18][P_TOK_NUM].state = 24;
	table[18][P_TOK_NUM].funcindex = 3;
	table[18][P_TOK_NUM].size = 1;

	table[18][P_TOK_RPAR].name = REDUCE;
	table[18][P_TOK_RPAR].state = 24;
	table[18][P_TOK_RPAR].funcindex = 3;
	table[18][P_TOK_RPAR].size = 1;

	table[20][P_TOK_MINUS].name = REDUCE;
	table[20][P_TOK_MINUS].state = 0;
	table[20][P_TOK_MINUS].funcindex = 0;
	table[20][P_TOK_MINUS].size = 4;

	table[20][P_TOK_PLUS].name = REDUCE;
	table[20][P_TOK_PLUS].state = 0;
	table[20][P_TOK_PLUS].funcindex = 0;
	table[20][P_TOK_PLUS].size = 4;

	table[20][P_TOK_LPAR].name = REDUCE;
	table[20][P_TOK_LPAR].state = 0;
	table[20][P_TOK_LPAR].funcindex = 0;
	table[20][P_TOK_LPAR].size = 4;

	table[20][P_TOK_NUM].name = REDUCE;
	table[20][P_TOK_NUM].state = 0;
	table[20][P_TOK_NUM].funcindex = 0;
	table[20][P_TOK_NUM].size = 4;

	table[20][P_TOK_RPAR].name = REDUCE;
	table[20][P_TOK_RPAR].state = 0;
	table[20][P_TOK_RPAR].funcindex = 0;
	table[20][P_TOK_RPAR].size = 4;

	table[21][P_TOK_MINUS].name = REDUCE;
	table[21][P_TOK_MINUS].state = 0;
	table[21][P_TOK_MINUS].funcindex = 0;
	table[21][P_TOK_MINUS].size = 4;

	table[21][P_TOK_PLUS].name = REDUCE;
	table[21][P_TOK_PLUS].state = 0;
	table[21][P_TOK_PLUS].funcindex = 0;
	table[21][P_TOK_PLUS].size = 4;

	table[21][P_TOK_LPAR].name = REDUCE;
	table[21][P_TOK_LPAR].state = 0;
	table[21][P_TOK_LPAR].funcindex = 0;
	table[21][P_TOK_LPAR].size = 4;

	table[21][P_TOK_NUM].name = REDUCE;
	table[21][P_TOK_NUM].state = 0;
	table[21][P_TOK_NUM].funcindex = 0;
	table[21][P_TOK_NUM].size = 4;

	table[21][P_TOK_RPAR].name = REDUCE;
	table[21][P_TOK_RPAR].state = 0;
	table[21][P_TOK_RPAR].funcindex = 0;
	table[21][P_TOK_RPAR].size = 4;

	table[22][P_TOK_LPAR].name = REDUCE;
	table[22][P_TOK_LPAR].state = 0;
	table[22][P_TOK_LPAR].funcindex = 0;
	table[22][P_TOK_LPAR].size = 4;

	table[22][P_TOK_NUM].name = REDUCE;
	table[22][P_TOK_NUM].state = 0;
	table[22][P_TOK_NUM].funcindex = 0;
	table[22][P_TOK_NUM].size = 4;

	table[22][P_TOK_RPAR].name = REDUCE;
	table[22][P_TOK_RPAR].state = 0;
	table[22][P_TOK_RPAR].funcindex = 0;
	table[22][P_TOK_RPAR].size = 4;

	table[24][P_TOK_LPAR].name = REDUCE;
	table[24][P_TOK_LPAR].state = 0;
	table[24][P_TOK_LPAR].funcindex = 0;
	table[24][P_TOK_LPAR].size = 4;

	table[24][P_TOK_NUM].name = REDUCE;
	table[24][P_TOK_NUM].state = 0;
	table[24][P_TOK_NUM].funcindex = 0;
	table[24][P_TOK_NUM].size = 4;

	table[24][P_TOK_RPAR].name = REDUCE;
	table[24][P_TOK_RPAR].state = 0;
	table[24][P_TOK_RPAR].funcindex = 0;
	table[24][P_TOK_RPAR].size = 4;

	table[25][P_TOK_LPAR].name = REDUCE;
	table[25][P_TOK_LPAR].state = 29;
	table[25][P_TOK_LPAR].funcindex = 3;
	table[25][P_TOK_LPAR].size = 1;

	table[25][P_TOK_MINUS].name = REDUCE;
	table[25][P_TOK_MINUS].state = 29;
	table[25][P_TOK_MINUS].funcindex = 3;
	table[25][P_TOK_MINUS].size = 1;

	table[25][P_TOK_NUM].name = REDUCE;
	table[25][P_TOK_NUM].state = 29;
	table[25][P_TOK_NUM].funcindex = 3;
	table[25][P_TOK_NUM].size = 1;

	table[25][P_TOK_PLUS].name = REDUCE;
	table[25][P_TOK_PLUS].state = 29;
	table[25][P_TOK_PLUS].funcindex = 3;
	table[25][P_TOK_PLUS].size = 1;

	table[25][P_TOK_RPAR].name = REDUCE;
	table[25][P_TOK_RPAR].state = 29;
	table[25][P_TOK_RPAR].funcindex = 3;
	table[25][P_TOK_RPAR].size = 1;

	table[26][P_TOK_LPAR].name = REDUCE;
	table[26][P_TOK_LPAR].state = 30;
	table[26][P_TOK_LPAR].funcindex = 3;
	table[26][P_TOK_LPAR].size = 1;

	table[26][P_TOK_MINUS].name = REDUCE;
	table[26][P_TOK_MINUS].state = 30;
	table[26][P_TOK_MINUS].funcindex = 3;
	table[26][P_TOK_MINUS].size = 1;

	table[26][P_TOK_NUM].name = REDUCE;
	table[26][P_TOK_NUM].state = 30;
	table[26][P_TOK_NUM].funcindex = 3;
	table[26][P_TOK_NUM].size = 1;

	table[26][P_TOK_PLUS].name = REDUCE;
	table[26][P_TOK_PLUS].state = 30;
	table[26][P_TOK_PLUS].funcindex = 3;
	table[26][P_TOK_PLUS].size = 1;

	table[26][P_TOK_RPAR].name = REDUCE;
	table[26][P_TOK_RPAR].state = 30;
	table[26][P_TOK_RPAR].funcindex = 3;
	table[26][P_TOK_RPAR].size = 1;

	table[27][P_TOK_LPAR].name = REDUCE;
	table[27][P_TOK_LPAR].state = 31;
	table[27][P_TOK_LPAR].funcindex = 3;
	table[27][P_TOK_LPAR].size = 1;

	table[27][P_TOK_NUM].name = REDUCE;
	table[27][P_TOK_NUM].state = 31;
	table[27][P_TOK_NUM].funcindex = 3;
	table[27][P_TOK_NUM].size = 1;

	table[27][P_TOK_RPAR].name = REDUCE;
	table[27][P_TOK_RPAR].state = 31;
	table[27][P_TOK_RPAR].funcindex = 3;
	table[27][P_TOK_RPAR].size = 1;

	table[28][P_TOK_LPAR].name = REDUCE;
	table[28][P_TOK_LPAR].state = 32;
	table[28][P_TOK_LPAR].funcindex = 3;
	table[28][P_TOK_LPAR].size = 1;

	table[28][P_TOK_NUM].name = REDUCE;
	table[28][P_TOK_NUM].state = 32;
	table[28][P_TOK_NUM].funcindex = 3;
	table[28][P_TOK_NUM].size = 1;

	table[28][P_TOK_RPAR].name = REDUCE;
	table[28][P_TOK_RPAR].state = 32;
	table[28][P_TOK_RPAR].funcindex = 3;
	table[28][P_TOK_RPAR].size = 1;

	table[29][P_TOK_LPAR].name = REDUCE;
	table[29][P_TOK_LPAR].state = 0;
	table[29][P_TOK_LPAR].funcindex = 0;
	table[29][P_TOK_LPAR].size = 3;

	table[29][P_TOK_MINUS].name = REDUCE;
	table[29][P_TOK_MINUS].state = 0;
	table[29][P_TOK_MINUS].funcindex = 0;
	table[29][P_TOK_MINUS].size = 3;

	table[29][P_TOK_NUM].name = REDUCE;
	table[29][P_TOK_NUM].state = 0;
	table[29][P_TOK_NUM].funcindex = 0;
	table[29][P_TOK_NUM].size = 3;

	table[29][P_TOK_PLUS].name = REDUCE;
	table[29][P_TOK_PLUS].state = 0;
	table[29][P_TOK_PLUS].funcindex = 0;
	table[29][P_TOK_PLUS].size = 3;

	table[29][P_TOK_RPAR].name = REDUCE;
	table[29][P_TOK_RPAR].state = 0;
	table[29][P_TOK_RPAR].funcindex = 0;
	table[29][P_TOK_RPAR].size = 3;

	table[30][P_TOK_LPAR].name = REDUCE;
	table[30][P_TOK_LPAR].state = 0;
	table[30][P_TOK_LPAR].funcindex = 0;
	table[30][P_TOK_LPAR].size = 3;

	table[30][P_TOK_MINUS].name = REDUCE;
	table[30][P_TOK_MINUS].state = 0;
	table[30][P_TOK_MINUS].funcindex = 0;
	table[30][P_TOK_MINUS].size = 3;

	table[30][P_TOK_NUM].name = REDUCE;
	table[30][P_TOK_NUM].state = 0;
	table[30][P_TOK_NUM].funcindex = 0;
	table[30][P_TOK_NUM].size = 3;

	table[30][P_TOK_PLUS].name = REDUCE;
	table[30][P_TOK_PLUS].state = 0;
	table[30][P_TOK_PLUS].funcindex = 0;
	table[30][P_TOK_PLUS].size = 3;

	table[30][P_TOK_RPAR].name = REDUCE;
	table[30][P_TOK_RPAR].state = 0;
	table[30][P_TOK_RPAR].funcindex = 0;
	table[30][P_TOK_RPAR].size = 3;

	table[31][P_TOK_LPAR].name = REDUCE;
	table[31][P_TOK_LPAR].state = 0;
	table[31][P_TOK_LPAR].funcindex = 0;
	table[31][P_TOK_LPAR].size = 3;

	table[31][P_TOK_NUM].name = REDUCE;
	table[31][P_TOK_NUM].state = 0;
	table[31][P_TOK_NUM].funcindex = 0;
	table[31][P_TOK_NUM].size = 3;

	table[31][P_TOK_RPAR].name = REDUCE;
	table[31][P_TOK_RPAR].state = 0;
	table[31][P_TOK_RPAR].funcindex = 0;
	table[31][P_TOK_RPAR].size = 3;

	table[32][P_TOK_LPAR].name = REDUCE;
	table[32][P_TOK_LPAR].state = 0;
	table[32][P_TOK_LPAR].funcindex = 0;
	table[32][P_TOK_LPAR].size = 3;

	table[32][P_TOK_NUM].name = REDUCE;
	table[32][P_TOK_NUM].state = 0;
	table[32][P_TOK_NUM].funcindex = 0;
	table[32][P_TOK_NUM].size = 3;

	table[32][P_TOK_RPAR].name = REDUCE;
	table[32][P_TOK_RPAR].state = 0;
	table[32][P_TOK_RPAR].funcindex = 0;
	table[32][P_TOK_RPAR].size = 3;

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

