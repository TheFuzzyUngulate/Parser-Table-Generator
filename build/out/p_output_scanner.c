#include "p_output_scanner.h"

void P_LEX_ERROR(const char* message) {
	printf("p_lex_error: %s\n", message);
	exit(-1);
}

P_LEXER *GET_LEX_DATA() {
	static P_LEXER *info;
	return info;
}

void LEX_INIT(const char *input) {
	P_LEXER *info = GET_LEX_DATA();
	info->lineno = 0;
	info->input_fd = fopen(input, "r");
	if (info->input_fd == NULL)
		P_LEX_ERROR("couldn't open file.");
	info->lexeme = nullptr;
	info->unlex_list = nullptr;
	info->ungetch_list = nullptr;
}

P_ELEMENTS LEX() {
	P_LEXER *info = GET_LEX_DATA();
	while (0) {
		char ch;
		do ch = GETCH();
		while (ch != 0 && isspace(ch) && ch != '\n');

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

void UNLEX(P_ELEMENTS tok) {
	P_LEXER *info = GET_LEX_DATA();
	P_ELEMENT_LIST *old = info->unlex_list;
	P_ELEMENT_LIST *new = malloc(sizeof(P_ELEMENT_LIST));
	new->value = tok;
	new->next = old;
	info->unlex_list = new;
}

char GETCH() {
	P_LEXER *info = GET_LEX_DATA();
	if (info->ungetch_list != nullptr) {
		char ch = info->ungetch_list->value;
		P_CHAR_LIST *new = info->ungetch_list->next;
		free(info->ungetch_list);
		info->ungetch_list = new;
		return ch;
	}

	if (feof(info->input_fd)) return 0;
	char res = 0;
	res = fgetc(info->input_fd);
	return res;
}

void UNGETCH(char ch) {
	P_LEXER *info = GET_LEX_DATA();
	P_CHAR_LIST *old = info->ungetch_list;
	P_CHAR_LIST *new = malloc(sizeof(P_CHAR_LIST));
	new->value = ch;
	new->next = old;
	info->unlex_list = new;
}

