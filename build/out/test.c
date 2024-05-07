#include "ptgparse.h"

void delim(int INDENT) {
    for (int i = 0; i < INDENT; ++i)
        for (int k = 0; k < 4; ++k) putchar(' ');
}

void ptg_node_print(ptg_node_t* tree, int INDENT)
{
    delim(INDENT);
    printf("%s:\n", tree->name);
    for (int i = 0; i < tree->ccount; ++i)
        ptg_node_print(tree->children[i], INDENT+1);
}

int main(int argc, char** argv)
{
    if (argc != 3) return EXIT_FAILURE;
    ptg_in  = fopen(argv[1], "r");
    ptg_out = fopen(argv[2], "w");
    ptg_node_print(ptg_parse(), 0);
}