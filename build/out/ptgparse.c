#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ptgparse.h"

typedef struct
m_stack
{
    int    count;
    size_t size;
    void*  content;
    int    capacity;
}
m_stack;
#define m_stack_tos(stack) (stack.content + (stack.size * (stack.count - 1)))

m_stack
_m_stack_init(size_t size)
{
    m_stack m;

    m.count    = 0;
    m.size     = size;
    m.capacity = 2;
    m.content  = malloc(m.capacity * m.size);

    return m;
}

#define m_stack_init(t) (_m_stack_init(sizeof(t)))

void m_stack_push(m_stack* m, void* item)
{
    if (m->count == m->capacity) {
        m->capacity = m->capacity * 3 / 2;
        m->content  = realloc(m->content, m->capacity * m->size);
    }
    memcpy(m->content + m->count, item, m->size);
    m->count++;
}

void* m_stack_pop(m_stack* m)
{
    if (m->count > 0) {
        void* item = malloc(m->size);
        memcpy(item, m->content + (m->count - 1), m->size);
        return item;
    } else return NULL;
}

void m_stack_clear(m_stack* m)
{
    if (m->count > 0) {
        m->capacity = 2;
        m->count = 0;
        m->content = realloc(m->content, m->capacity * m->size);
    }
}

// static FILE* ptg_infile = NULL;

typedef struct
ptg_sc
{
    unsigned int line;     // current line number
    unsigned int column;   // current column number
    struct {
        m_stack  lexeme;   // current lexeme
        m_stack  chbuf;    // unget buffer
        m_stack  tokbuf;   // unlex buffer
        ptg_enum lasttok;  // last token returned by lex
    } prog;
    struct {
        m_stack seeks;     // saved positions for seek
        m_stack answers;   // saved answers for answer checking
        m_stack counters;  // saved counters for future updates
    } check;
}
ptg_sc;

static ptg_sc* sc = NULL;

char scan() {
    char ch;
    if (sc->prog.chbuf.count > 0) {
        ch = *(char*)m_stack_pop(&(sc->prog.chbuf));
    } else {
        ch = fgetc(ptg_infile);
        if (ch == EOF) 
            return EOF;
    }
    m_stack_push(&(sc->prog.lexeme), &ch);
    return ch;
}

void save_pos() {
    int tl = ftell(ptg_infile);
    m_stack_push(&(sc->check.seeks), &tl);
}

char load_pos() {
    if (sc->check.seeks.count > 0) {
        int p  = *(int*)m_stack_pop(&(sc->check.seeks));
        int fs = fseek( ptg_infile, p, SEEK_SET );
        if (!fs) {
            fprintf(stderr, "error: fseek failed.\n");
            exit(EXIT_FAILURE);
        }
        return scan();
    } else {
        fprintf(stderr, "error: no seek positions have been saved.\n");
        exit(EXIT_FAILURE);
    }
}

char unscan(char ch) {
    m_stack_push(&(sc->prog.chbuf), &ch);
}

void save_bool(bool q) {
    m_stack_push(&(sc->check.answers), &q);
}

bool load_bool() {
    if (sc->check.answers.count > 0) {
        return *(bool*)m_stack_pop(&(sc->check.answers));
    } else {
        fprintf(stderr, "error: no answers have been saved.\n");
        exit(EXIT_FAILURE);
    }
}

void new_counter() {
    int q = 0;
    m_stack_push(&(sc->check.counters), &q);
}

int count() {
    if (sc->check.counters.count > 0) {
        return *(int*)m_stack_pop(&(sc->check.counters));
    } else {
        fprintf(stderr, "error: no counters have been saved.\n");
        exit(EXIT_FAILURE);
    }
}

int inc_counter() {
    if (sc->check.counters.count > 0) {
        ++(*(int*)m_stack_tos(sc->check.counters));
    } else {
        fprintf(stderr, "error: no counters have been saved.\n");
        exit(EXIT_FAILURE);
    }
}

#define ptg_tokret(val) do {\
    sc->prog.lasttok = (val);\
    return (val);\
} while(0);

char* ptg_lexeme() {
    char* res = (char*)malloc((sc->prog.lexeme.count + 1) * sizeof(char));
    memcpy(res, sc->prog.lexeme.content, sc->prog.lexeme.count * sizeof(char));
    res[sc->prog.lexeme.count] = '\0';
    return res;
}

ptg_enum
ptg_lex()
{
    int ch;
    if (sc == NULL)
    {
        sc = (ptg_sc*)malloc(sizeof(ptg_sc));

        sc->line   = 0;
        sc->column = 0;

        sc->prog.lexeme = m_stack_init(char);
        sc->prog.chbuf  = m_stack_init(char);
        sc->prog.tokbuf = m_stack_init(ptg_enum);

        sc->check.seeks    = m_stack_init(int);
        sc->check.answers  = m_stack_init(bool);
        sc->check.counters = m_stack_init(int);
    }

    if (sc->prog.lexeme.count > 0) {
        m_stack_clear(&(sc->prog.lexeme));
    }

    if (sc->prog.tokbuf.count > 0) {
        ptg_enum tok = *(ptg_enum*)m_stack_pop(&(sc->prog.tokbuf));
        return tok;
    }

    while (1)
    {
		ch = scan();

		save_pos();
		do {
			save_bool(ch == '{');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch >= 'A' && ch <= 'Z');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_pos();
			new_counter();
			while (true) {
				save_bool(ch == '}');
				ch = scan();
				if (!load_bool()) {
					ch = load_pos();
				break;
					} else {
					save_pos();
					inc_counter();
				}
			} save_bool(count() > 0);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_ELEMENT)
		else ch = load_pos();

		save_pos();
		do {
			save_bool(ch == '\"');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_pos();
			save_pos();
			new_counter();
			while (true) {
				save_bool(ch == '\"');
				ch = scan();
				if (!load_bool()) {
					ch = load_pos();
				break;
					} else {
					save_pos();
					inc_counter();
				}
			} save_bool(count() > 0);
			if (!load_bool()) {
				ch = load_pos();
					do {
						save_bool(ch == '\\');
						ch = scan();
						if (!load_bool()) {
							save_bool(false);
							break;
						}
						save_bool(ch == '\"');
						ch = scan();
						if (!load_bool()) {
							save_bool(false);
							break;
						}
						save_bool(true);
					} while (0);
			} else save_bool(true);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch == '\"');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_STRING)
		else ch = load_pos();

		save_pos();
		save_bool(ch == ':');
		ch = scan();
		if (load_bool())
			ptg_tokret(P_TOK_COLON)
		else ch = load_pos();

		save_pos();
		do {
			save_bool(ch == '{');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch >= 'a' && ch <= 'z');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_pos();
			new_counter();
			while (true) {
				save_bool(ch == '}');
				ch = scan();
				if (!load_bool()) {
					ch = load_pos();
				break;
					} else {
					save_pos();
					inc_counter();
				}
			} save_bool(count() > 0);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_ATTRIBUTE)
		else ch = load_pos();

		save_pos();
		do {
			save_bool(ch == '@');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch == '{');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch >= '0' && ch <= '9');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_pos();
			new_counter();
			while (true) {
				save_bool(ch == '}');
				ch = scan();
				if (!load_bool()) {
					ch = load_pos();
				break;
					} else {
					save_pos();
					inc_counter();
				}
			} save_bool(count() > 0);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_ADDRESS)
		else ch = load_pos();

		save_pos();
		save_bool(ch == '[');
		ch = scan();
		if (load_bool())
			ptg_tokret(P_TOK_LBRACK)
		else ch = load_pos();

		save_pos();
		save_bool(ch == ']');
		ch = scan();
		if (load_bool())
			ptg_tokret(P_TOK_RBRACK)
		else ch = load_pos();

		save_pos();
		save_bool(ch == ',');
		ch = scan();
		if (load_bool())
			ptg_tokret(P_TOK_COMMA)
		else ch = load_pos();

		save_pos();
		do {
			save_bool(ch == '#');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch == '{');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			do {
				save_bool(ch >= 'A' && ch <= 'Z');
				ch = scan();
				if (load_bool()) {
					save_bool(true);
					break;
				}
				save_bool(ch >= 'a' && ch <= 'z');
				ch = scan();
				if (load_bool()) {
					save_bool(true);
					break;
				}
				save_bool(false);
			} while (0);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_pos();
			new_counter();
			while (true) {
				save_bool(ch == '}');
				ch = scan();
				if (!load_bool()) {
					ch = load_pos();
				break;
					} else {
					save_pos();
					inc_counter();
				}
			} save_bool(count() > 0);
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_DIRECTIVE)
		else ch = load_pos();

		save_pos();
		do {
			save_bool(ch == '.');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch == '.');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(ch == '.');
			ch = scan();
			if (!load_bool()) {
				save_bool(false);
				break;
			}
			save_bool(true);
		} while (0);
		if (load_bool())
			ptg_tokret(P_TOK_ELLIPSIS)
		else fprintf(stderr, "invalid char \"%c\"found\n", ch);
    }
}

void ptg_unlex(ptg_enum tok) {
    m_stack_push(&(sc->prog.tokbuf), &tok);
}

#define P_RULE_COUNT 26
#define P_STATE_COUNT 40
#define P_ELEMENT_COUNT 27

#define PTG_GOTO   1
#define PTG_ERROR  0
#define PTG_SHIFT  2
#define PTG_REDUCE 3
#define PTG_ACCEPT 4

typedef struct ptg_pdata {
    int action;
    union { 
        int shift;
        int sgoto;
        char accept;
        char error;
        struct { 
            int rule;
            int count;
            ptg_enum lhs_tok;
        } reduce; 
    } op;
} ptg_pdata_t;

static ptg_pdata_t *ptg_ptable = NULL;

#define ptg_table_next(state, token) (*(ptg_ptable + ((state) * P_ELEMENT_COUNT) + (token)))

static inline void
ptg_table_set(int state, ptg_enum token, ptg_pdata_t data)
{
    ptg_pdata_t old = ptg_table_next(state, token);
    if (old.action != PTG_ERROR) {
        if (old.action == PTG_SHIFT && data.action == PTG_REDUCE) {
            fprintf(stderr, stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, ptg_enum_string(token));
            fprintf(stderr, stderr, "opting to shift...\n");
            return;
        }
        if (old.action == PTG_REDUCE && data.action == PTG_SHIFT) {
            fprintf(stderr, stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, ptg_enum_string(token));
            fprintf(stderr, stderr, "opting to shift...\n");
        }
        if (old.action == PTG_REDUCE && data.action == PTG_REDUCE) {
            fprintf(stderr, stderr, "error: reduce-reduce conflict at (state#%d, %s)\n", state, ptg_enum_string(token));
            exit(EXIT_FAILURE);
        }
    }

    ptg_table_next(state, token) = data;
}

static inline void
ptg_ptable_init()
{
    ptg_ptable = (ptg_pdata_t*)calloc(P_STATE_COUNT * P_ELEMENT_COUNT, sizeof(ptg_pdata_t));

	ptg_table_set(0, P_TOK_DIRECTIVE, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 1});
	ptg_table_set(0, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 2});
	ptg_table_set(0, P_LIT_directive, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 3});
	ptg_table_set(0, P_LIT_item, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 4});
	ptg_table_set(0, P_LIT_start, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 5});
	ptg_table_set(0, P_LIT_struct, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 6});
	ptg_table_set(1, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 7});
	ptg_table_set(1, P_TOK_STRING, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 8});
	ptg_table_set(1, P_LIT_diritem, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 9});
	ptg_table_set(2, P_TOK_COLON, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 10});
	ptg_table_set(4, P_TOK_DIRECTIVE, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 1});
	ptg_table_set(4, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 2});
	ptg_table_set(4, P_LIT_directive, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 3});
	ptg_table_set(4, P_LIT_item, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 11});
	ptg_table_set(4, P_LIT_start_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 12});
	ptg_table_set(4, P_LIT_struct, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 6});
	ptg_table_set(9, P_TOK_COMMA, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 13});
	ptg_table_set(9, P_LIT_directive_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 14});
	ptg_table_set(10, P_TOK_ATTRIBUTE, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 15});
	ptg_table_set(10, P_LIT_attribute, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 16});
	ptg_table_set(10, P_LIT_struct_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 17});
	ptg_table_set(11, P_TOK_DIRECTIVE, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 1});
	ptg_table_set(11, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 2});
	ptg_table_set(11, P_LIT_directive, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 3});
	ptg_table_set(11, P_LIT_item, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 11});
	ptg_table_set(11, P_LIT_start_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 18});
	ptg_table_set(11, P_LIT_struct, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 6});
	ptg_table_set(13, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 7});
	ptg_table_set(13, P_TOK_STRING, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 8});
	ptg_table_set(13, P_LIT_diritem, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 19});
	ptg_table_set(15, P_TOK_COLON, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 20});
	ptg_table_set(16, P_TOK_ATTRIBUTE, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 15});
	ptg_table_set(16, P_LIT_attribute, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 16});
	ptg_table_set(16, P_LIT_struct_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 21});
	ptg_table_set(19, P_TOK_COMMA, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 13});
	ptg_table_set(19, P_LIT_directive_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 22});
	ptg_table_set(20, P_TOK_ADDRESS, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 23});
	ptg_table_set(20, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 24});
	ptg_table_set(20, P_TOK_ELLIPSIS, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 25});
	ptg_table_set(20, P_TOK_STRING, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 26});
	ptg_table_set(20, P_LIT_feature, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 27});
	ptg_table_set(20, P_LIT_literal, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 28});
	ptg_table_set(20, P_LIT_litlist, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 29});
	ptg_table_set(24, P_TOK_LBRACK, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 30});
	ptg_table_set(24, P_LIT_featlist, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 31});
	ptg_table_set(24, P_LIT_feature_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 32});
	ptg_table_set(28, P_TOK_ADDRESS, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 23});
	ptg_table_set(28, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 24});
	ptg_table_set(28, P_TOK_ELLIPSIS, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 25});
	ptg_table_set(28, P_TOK_STRING, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 26});
	ptg_table_set(28, P_LIT_feature, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 27});
	ptg_table_set(28, P_LIT_literal, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 28});
	ptg_table_set(28, P_LIT_litlist, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 33});
	ptg_table_set(30, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 34});
	ptg_table_set(34, P_TOK_COMMA, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 35});
	ptg_table_set(34, P_LIT_featlist_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 36});
	ptg_table_set(35, P_TOK_ELEMENT, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 37});
	ptg_table_set(36, P_TOK_RBRACK, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 38});
	ptg_table_set(37, P_TOK_COMMA, (ptg_pdata_t){ .action = PTG_SHIFT, .op.shift = 35});
	ptg_table_set(37, P_LIT_featlist_BAR, (ptg_pdata_t){ .action = PTG_GOTO, .op.sgoto = 39});
	ptg_table_set(3, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {4, 1, P_LIT_item}});
	ptg_table_set(3, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {4, 1, P_LIT_item}});
	ptg_table_set(3, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {4, 1, P_LIT_item}});
	ptg_table_set(4, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {2, 0, P_LIT_start_BAR}});
	ptg_table_set(5, P_TOK_END, (ptg_pdata_t){.action = PTG_ACCEPT, .op.accept = 0});
	ptg_table_set(6, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {3, 1, P_LIT_item}});
	ptg_table_set(6, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {3, 1, P_LIT_item}});
	ptg_table_set(6, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {3, 1, P_LIT_item}});
	ptg_table_set(7, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {25, 1, P_LIT_diritem}});
	ptg_table_set(7, P_TOK_COMMA, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {25, 1, P_LIT_diritem}});
	ptg_table_set(7, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {25, 1, P_LIT_diritem}});
	ptg_table_set(7, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {25, 1, P_LIT_diritem}});
	ptg_table_set(8, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {24, 1, P_LIT_diritem}});
	ptg_table_set(8, P_TOK_COMMA, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {24, 1, P_LIT_diritem}});
	ptg_table_set(8, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {24, 1, P_LIT_diritem}});
	ptg_table_set(8, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {24, 1, P_LIT_diritem}});
	ptg_table_set(9, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(9, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(9, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(10, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(10, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(10, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(11, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {2, 0, P_LIT_start_BAR}});
	ptg_table_set(12, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {0, 2, P_LIT_start}});
	ptg_table_set(14, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {21, 3, P_LIT_directive}});
	ptg_table_set(14, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {21, 3, P_LIT_directive}});
	ptg_table_set(14, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {21, 3, P_LIT_directive}});
	ptg_table_set(16, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(16, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(16, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {7, 0, P_LIT_struct_BAR}});
	ptg_table_set(17, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {5, 3, P_LIT_struct}});
	ptg_table_set(17, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {5, 3, P_LIT_struct}});
	ptg_table_set(17, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {5, 3, P_LIT_struct}});
	ptg_table_set(18, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {1, 2, P_LIT_start_BAR}});
	ptg_table_set(19, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(19, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(19, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {23, 0, P_LIT_directive_BAR}});
	ptg_table_set(20, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(20, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(20, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(20, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(21, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {6, 2, P_LIT_struct_BAR}});
	ptg_table_set(21, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {6, 2, P_LIT_struct_BAR}});
	ptg_table_set(21, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {6, 2, P_LIT_struct_BAR}});
	ptg_table_set(22, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {22, 3, P_LIT_directive_BAR}});
	ptg_table_set(22, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {22, 3, P_LIT_directive_BAR}});
	ptg_table_set(22, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {22, 3, P_LIT_directive_BAR}});
	ptg_table_set(23, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(23, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {14, 1, P_LIT_literal}});
	ptg_table_set(24, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(24, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {17, 0, P_LIT_feature_BAR}});
	ptg_table_set(25, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {9, 1, P_LIT_litlist}});
	ptg_table_set(25, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {9, 1, P_LIT_litlist}});
	ptg_table_set(25, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {9, 1, P_LIT_litlist}});
	ptg_table_set(25, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {9, 1, P_LIT_litlist}});
	ptg_table_set(26, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(26, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {13, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(27, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {12, 1, P_LIT_literal}});
	ptg_table_set(28, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(28, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(28, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(28, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {11, 0, P_LIT_litlist}});
	ptg_table_set(29, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {8, 3, P_LIT_attribute}});
	ptg_table_set(29, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {8, 3, P_LIT_attribute}});
	ptg_table_set(29, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {8, 3, P_LIT_attribute}});
	ptg_table_set(29, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {8, 3, P_LIT_attribute}});
	ptg_table_set(31, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(31, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {16, 1, P_LIT_feature_BAR}});
	ptg_table_set(32, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(32, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {15, 2, P_LIT_feature}});
	ptg_table_set(33, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {10, 2, P_LIT_litlist}});
	ptg_table_set(33, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {10, 2, P_LIT_litlist}});
	ptg_table_set(33, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {10, 2, P_LIT_litlist}});
	ptg_table_set(33, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {10, 2, P_LIT_litlist}});
	ptg_table_set(34, P_TOK_RBRACK, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {20, 0, P_LIT_featlist_BAR}});
	ptg_table_set(37, P_TOK_RBRACK, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {20, 0, P_LIT_featlist_BAR}});
	ptg_table_set(38, P_TOK_END, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_ADDRESS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_ATTRIBUTE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_DIRECTIVE, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_ELEMENT, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_ELLIPSIS, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(38, P_TOK_STRING, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {18, 4, P_LIT_featlist}});
	ptg_table_set(39, P_TOK_RBRACK, (ptg_pdata_t){.action = PTG_REDUCE, .op.reduce = {19, 3, P_LIT_featlist_BAR}});
}

typedef struct
ptg_node
{
    char* name;
    ptg_enum tok;
    int ccount;
    int ccapacity;
    struct ptg_node **children;
}
ptg_node_t;

ptg_node_t*
ptg_node_new(char* name, ptg_enum tok)
{
    ptg_node_t* node = (ptg_node_t*)malloc(sizeof(ptg_node_t));

    node->name      = name;
    node->ccount    = 0;
    node->tok       = tok;
    node->ccapacity = 2;
    node->children  = (ptg_node_t**)malloc(node->ccapacity * sizeof(ptg_node_t*));

    return node;
}

void ptg_node_addchild(ptg_node_t* node, ptg_node_t* child)
{
    if (node->ccount == node->ccapacity) {
        node->ccapacity = node->ccapacity * 3 / 2;
        node->children  = (ptg_node_t**)realloc(node->children, node->ccapacity * sizeof(ptg_node_t*));
    }

    node->children[node->ccount] = child;
    node->ccount++;
}

void ptg_node_print(ptg_node_t* node, int indent)
{
    int i;

    for (i = 0; i < indent; ++i) putchar(' ');
    printf("%s: %s\n", node->name, ptg_enum_string(node->tok));
    for (i = 0; i < node->ccount; ++i)
        ptg_node_print(node->children[i], indent+1);
}

ptg_node_t* ptg_parse()
{
    char a;
    int state;
    ptg_enum tos;
    m_stack nodes;
    m_stack states;
    m_stack symbols;
    ptg_pdata_t next;
    ptg_node_t *leaf;

    ptg_ptable_init();

    state   = 0;
    states  = m_stack_init(int);
    symbols = m_stack_init(ptg_enum);
    nodes   = m_stack_init(ptg_node_t*);
    
    m_stack_push(&states, &state);

    while (1)
    {
        tos   = ptg_lex();
        state = *(int*)m_stack_tos(states);
        next  = ptg_table_next(state, tos);

        switch (next.action)
        {
            case PTG_SHIFT:
                m_stack_push(&symbols, &tos);
                m_stack_push(&states, &(next.op.shift));
                leaf = ptg_node_new(ptg_getlexeme(), tos);
                m_stack_push(&nodes, &leaf);
                break;

            case PTG_REDUCE:
                m_stack_pop(&states);
                leaf = ptg_node_new(NULL, tos);
                for (int i = 0; i < next.op.reduce.count; ++i) {
                    ptg_node_t** sub = (ptg_node_t**)m_stack_pop(&nodes);
                    if (sub == NULL) {
                        fprintf(stderr, stderr, "stack out of nodes.\n");
                        exit(EXIT_FAILURE);
                    }
                    ptg_node_addchild(leaf, *sub);
                }
                m_stack_push(&nodes, &leaf);
                m_stack_push(&states, &(next.op.reduce.rule));
                break;

            case PTG_GOTO:
                m_stack_push(&states, &(next.op.sgoto));
                break;

            case PTG_ACCEPT:
                leaf = *(ptg_node_t**)m_stack_pop(&nodes);
                return leaf;

            case PTG_ERROR:
                fprintf(stderr, stderr, "invalid token \"%s\".\n", ptg_enum_string(tos));
                exit(EXIT_FAILURE);
        }
    }
}

#undef ptg_table_next
