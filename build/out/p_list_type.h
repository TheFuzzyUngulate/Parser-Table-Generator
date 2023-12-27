#ifndef P_LIST_TYPE_H
#define P_LIST_TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct List {
	int o_size;
	int o_count;
	void **o_ptr;
	int o_capacity;
} P_LIST_TYPE;
P_LIST_TYPE P_LIST_INIT(unsigned int size);
void P_LIST_PUSH(P_LIST_TYPE *list, void *i);
void P_LIST_POP(P_LIST_TYPE *list, unsigned int count, void **buffer);
void *P_LIST_TOP(P_LIST_TYPE *list);
void p_list_error(const char* msg);

#endif