#include "p_output_scanner.h"

P_LEXER LEX_INIT(const char *input) {
	P_LEXER info;
	info.lineno = 0;
	info.input_fd = fopen(input, "r");
	if (info.input_fd == NULL)
		P_LEX_ERROR("couldn't open file.");
	info.lexeme = malloc(sizeof(char));
	memset(info.lexeme, 0, sizeof(char));
	info.unlex_list = nullptr;
	info.ungetch_list = nullptr;
	return info;
}

P_ELEMENTS LEX(P_LEXER *info) {
	while (1) {
		char ch;
		do ch = GETCH(info);
		while (ch != 0 && isspace(ch) && ch != '\n');

		free(info->lexeme);
		info->lexeme = malloc(sizeof(char));
		memset(info->lexeme, 0, sizeof(char));

		switch (ch) {
			case 0:
				return P_TOK_END;

			case '\n':
				info->lineno += 1;
				break;

			//Implement others here!!
		}
	}
}

void UNLEX(P_LEXER *info, P_ELEMENTS tok) {
	P_ELEMENT_LIST *old = info->unlex_list;
	P_ELEMENT_LIST *new = malloc(sizeof(P_ELEMENT_LIST));
	new->value = tok;
	new->next = old;
	info->unlex_list = new;
}

char GETCH(P_LEXER *info) {
	if (info->ungetch_list != nullptr) {
		char ch = info->ungetch_list->value;
		P_CHAR_LIST *new = info->ungetch_list->next;
		free(info->ungetch_list);
		info->ungetch_list = new;
		return ch;
	}

	char res = getc(info->input_fd);
	return res == EOF ? 0 : res;
}

void UNGETCH(P_LEXER *info, char ch) {
	P_CHAR_LIST *old = info->ungetch_list;
	P_CHAR_LIST *new = malloc(sizeof(P_CHAR_LIST));
	new->value = ch;
	new->next = old;
	info->ungetch_list = new;
}

