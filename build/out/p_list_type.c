#include "p_list_type.h"

P_LIST_TYPE P_LIST_INIT(unsigned int size) {
	void **ptr = malloc(10 * sizeof(void*));
	memset(ptr, 0, 10 * sizeof(void*));
	P_LIST_TYPE retstr = {size, 0, ptr, 10};
	return retstr;
}

void p_list_error(const char* msg) {
	fprintf(stderr, "p_list_error: %s", msg);
	exit(-1);
}

void P_LIST_PUSH(P_LIST_TYPE *list, void *i) {
	if (list->o_count < list->o_capacity)
		list->o_ptr[list->o_count++] = i;
	else if (list->o_count == list->o_capacity) {
		void **new = malloc(list->o_capacity * 2 * list->o_size);
		for (int i = 0; i < list->o_count; ++i)
			new[i] = list->o_ptr[i];
		free(list->o_ptr);
		list->o_ptr = new;
		list->o_count++;
		list->o_capacity = list->o_capacity * 2;
	}
else p_list_error("error while pushing to P_LIST_TYPE.");
}

void P_LIST_POP(P_LIST_TYPE *list, unsigned int count, void **buffer) {
	if (count > list->o_count)
		p_list_error("error while popping from P_LIST_TYPE.");
	for (int i = 0; i < count; ++i)
		buffer[(list->o_count-1) - i] = list->o_ptr[i];
	list->o_count -= count;
}

void *P_LIST_TOP(P_LIST_TYPE *list) {
	return list->o_ptr[list->o_count-1];
}

