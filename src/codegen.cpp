#include "../include/gen/codegen.hpp"

void CodeGenerator::genPrereqs() {
    auto file = std::ofstream();
    file.open("out/p_list_type.h");
    file << "#ifndef P_LIST_TYPE_H\n#define P_LIST_TYPE_H\n\n";
    file << "#include <stdio.h>\n#include <stdlib.h>\n#include <unistd.h>\n\n";
    file << "typedef struct List {\n\tint o_size;\n\tint o_count;\n\tvoid **o_ptr;\n\tint o_capacity;\n} P_LIST_TYPE;\n";
    file << "P_LIST_TYPE P_LIST_INIT(unsigned int size);\n";
    file << "void P_LIST_PUSH(P_LIST_TYPE *list, void *i);\n";
    file << "void P_LIST_POP(P_LIST_TYPE *list, unsigned int count, void **buffer);\n";
    file << "void *P_LIST_TOP(P_LIST_TYPE *list);\n";
    file << "void p_list_error(const char* msg);\n\n";
    file << "#endif";
    file.close();
    // create file for handling lists
    file.open("out/p_list_type.c");
    file << "#include \"p_list_type.h\"\n\n";
    // init function
    file << "P_LIST_TYPE P_LIST_INIT(unsigned int size) {\n";
    file << "\tvoid **ptr = malloc(10 * sizeof(void*));\n";
    file << "\tmemset(ptr, 0, 10 * sizeof(void*));\n";
    file << "\tP_LIST_TYPE retstr = {size, 0, ptr, 10};\n";
    file << "\treturn retstr;\n";
    file << "}\n\n";
    // error function
    file << "void p_list_error(const char* msg) {\n";
    file << "\tfprintf(stderr, \"p_list_error: %s\", msg);\n";
    file << "\texit(-1);\n";
    file << "}\n\n";
    // push function
    file << "void P_LIST_PUSH(P_LIST_TYPE *list, void *i) {\n";
    file << "\tif (list->o_count < list->o_capacity)\n";
    file << "\t\tlist->o_ptr[list->o_count++] = i;\n";
    file << "\telse if (list->o_count == list->o_capacity) {\n";
    file << "\t\tvoid **new = malloc(list->o_capacity * 2 * list->o_size);\n";
    file << "\t\tfor (int i = 0; i < list->o_count; ++i)\n\t\t\tnew[i] = list->o_ptr[i];\n";
    file << "\t\tfree(list->o_ptr);\n";
    file << "\t\tlist->o_ptr = new;\n";
    file << "\t\tlist->o_count++;\n";
    file << "\t\tlist->o_capacity = list->o_capacity * 2;\n";
    file << "\t}\nelse p_list_error(\"error while pushing to P_LIST_TYPE.\");\n";
    file << "}\n\n";
    // pop function
    file << "void P_LIST_POP(P_LIST_TYPE *list, unsigned int count, void **buffer) {\n";
    file << "\tif (count > list->o_count)\n\t\tp_list_error(\"error while popping from P_LIST_TYPE.\");\n";
    file << "\tfor (int i = 0; i < count; ++i)\n\t\tbuffer[(list->o_count-1) - i] = list->o_ptr[i];\n";
    file << "\tlist->o_count -= count;\n";
    file << "}\n\n";
    // top function
    file << "void *P_LIST_TOP(P_LIST_TYPE *list) {\n";
    file << "\treturn list->o_ptr[list->o_count-1];\n}\n\n";
}

void CodeGenerator::genScannerFiles() {
    // create and open scanner header file
    auto file = std::ofstream();
    file.open("out/p_output_scanner.h");
    // define header information
    file << "#ifndef P_OUTPUT_SCANNER_H\n#define P_OUTPUT_SCANNER_H\n#pragma once\n\n";
    file << "#include <stdio.h>\n#include <unistd.h>\n#include <stdlib.h>\n\n";
    file << "#define nullptr ((void*)0)\n\n";
    // add token enum declaration
    file << "typedef enum P_ELEMENTS {\n";
    for (int i = 0; i < _elements.size(); ++i)
        file << "\t" << _elementtoks[_elements[i]] << ((i != _elements.size()-1) ? "," : "") << "\n";
    file << "} P_ELEMENTS;\n\n";
    // create unlex list type
    file << "typedef struct P_ELEMENT_LIST {\n";
    file << "\tP_ELEMENTS value;\n";
    file << "\tP_ELEMENT_LIST *next;\n} P_ELEMENT_LIST;\n\n";
    // create ungetch list type
    file << "typedef struct P_CHAR_LIST {\n";
    file << "\tchar value;\n";
    file << "\tP_CHAR_LIST *next;\n} P_CHAR_LIST;\n\n";
    // create lexer struct containing lexing info
    file << "typedef struct P_LEXER {\n";
    file << "\tint lineno;\n";
    file << "\tFILE* input_fd;\n";
    file << "\tconst char *lexeme;\n";
    file << "\tP_CHAR_LIST *ungetch_list;\n";
    file << "\tP_ELEMENT_LIST *unlex_list;\n} P_LEXER;\n\n";
    // create functions "lex", "unlex", "getch", and "ungetch"; as well as "lex_init" that does bookkeeping
    file << "char GETCH();\n";
    file << "void UNGETCH(char ch);\n";
    file << "void UNLEX(P_ELEMENTS tok);\n";
    file << "P_ELEMENTS LEX();\n";
    file << "void LEX_INIT(const char *input);\n";
    file << "P_LEXER *GET_LEX_DATA();\n\n";
    file << "void P_LEX_ERROR(const char* message);\n\n";
    // finalize and close file
    file << "#endif";
    file.close();
    file.open("out/p_output_scanner.c");
    // define header information
    file << "#include \"p_output_scanner.h\"\n\n";
    // work on P_LEX_ERROR
    file << "void P_LEX_ERROR(const char* message) {\n";
    file << "\tprintf(\"p_lex_error: %s\\n\", message);\n";
    file << "\texit(-1);\n}\n\n";
    // work on GET_LEX_DATA
    file << "P_LEXER *GET_LEX_DATA() {\n";
    file << "\tstatic P_LEXER *info;\n";
    file << "\treturn info;\n}\n\n";
    // work on LEX_INIT
    file << "void LEX_INIT(const char *input) {\n";
    file << "\tP_LEXER *info = GET_LEX_DATA();\n";
    file << "\tinfo->lineno = 0;\n";
    file << "\tinfo->input_fd = fopen(input, \"r\");\n";
    file << "\tif (info->input_fd == NULL)\n\t\tP_LEX_ERROR(\"couldn't open file.\");\n";
    file << "\tinfo->lexeme = nullptr;\n";
    file << "\tinfo->unlex_list = nullptr;\n";
    file << "\tinfo->ungetch_list = nullptr;\n";
    file << "}\n\n";
    // work on LEX
    file << "P_ELEMENTS LEX() {\n";
    file << "\tP_LEXER *info = GET_LEX_DATA();\n";
    file << "\twhile (0) {\n";
    file << "\t\tchar ch;\n";
    file << "\t\tdo ch = GETCH();\n";
    file << "\t\twhile (ch != 0 && isspace(ch) && ch != '\\n');\n\n";
    file << "\t\tswitch (ch) {\n";
    file << "\t\t\tcase 0:\n";
    file << "\t\t\t\treturn P_TOK_END;\n\n";
    file << "\t\t\tcase '\\n':\n";
    file << "\t\t\t\tinfo->lineno += 1;\n";
    file << "\t\t\t\tbreak;\n\n";
    file << "\t\t\t//Implement others here!!\n";
    file << "\t\t}\n";
    file << "\t}\n";
    file << "}\n\n";
    // work on UNLEX
    file << "void UNLEX(P_ELEMENTS tok) {\n";
    file << "\tP_LEXER *info = GET_LEX_DATA();\n";
    file << "\tP_ELEMENT_LIST *old = info->unlex_list;\n";
    file << "\tP_ELEMENT_LIST *new = malloc(sizeof(P_ELEMENT_LIST));\n";
    file << "\tnew->value = tok;\n";
    file << "\tnew->next = old;\n";
    file << "\tinfo->unlex_list = new;\n";
    file << "}\n\n";
    // work on GETCH
    file << "char GETCH() {\n";
    file << "\tP_LEXER *info = GET_LEX_DATA();\n";
    file << "\tif (info->ungetch_list != nullptr) {\n";
    file << "\t\tchar ch = info->ungetch_list->value;\n";
    file << "\t\tP_CHAR_LIST *new = info->ungetch_list->next;\n";
    file << "\t\tfree(info->ungetch_list);\n";
    file << "\t\tinfo->ungetch_list = new;\n";
    file << "\t\treturn ch;\n";
    file << "\t}\n\n";
    file << "\tif (feof(info->input_fd)) return 0;\n";
    file << "\tchar res = 0;\n";
    file << "\tres = fgetc(info->input_fd);\n";
    file << "\treturn res;\n";
    file << "}\n\n";
    // work on UNGETCH
    file << "void UNGETCH(char ch) {\n";
    file << "\tP_LEXER *info = GET_LEX_DATA();\n";
    file << "\tP_CHAR_LIST *old = info->ungetch_list;\n";
    file << "\tP_CHAR_LIST *new = malloc(sizeof(P_CHAR_LIST));\n";
    file << "\tnew->value = ch;\n";
    file << "\tnew->next = old;\n";
    file << "\tinfo->unlex_list = new;\n";
    file << "}\n\n";
    // finalize and close file
    file.close();
}

void CodeGenerator::genParserFiles() {
    auto file = std::ofstream();
    file.open("out/p_output_parser.h");
    // add header information
    file << "#ifndef P_OUTPUT_PARSER_H\n#define P_OUTPUT_PARSER_H\n#pragma once\n\n";
    file << "#include <stdio.h>\n#include <unistd.h>\n#include <stdlib.h>\n#include \"p_list_type.h\"\n#include \"p_output_scanner.h\"\n\n";
    file << "#define nullptr ((void*)0)\n\n";
    // add macros for sizes
    file << "#define P_RULE_COUNT " << _hf->getRuleCount() << "\n";
    file << "#define P_STATE_COUNT " << _hf->getStateCount() << "\n";
    file << "#define P_ELEMENT_COUNT " << _elements.size() << "\n";
    file << "typedef void (*p_callback)(struct P_PARSE_CLASS* lst);\n\n";
    // add parser struct, to be implemented by the user
    // however, the parser struct ought to have some default values (array of pointers to struct (so, pointer of pointers), name, child_count, etc)
    file << "typedef struct P_PARSE_CLASS {\n";
    file << "\tP_ELEMENTS type;\n\tconst char* name;\n\tP_PARSE_CLASS **children;\n\tint child_count;\n";
    file << "} P_PARSE_CLASS;\n\n";
    // add function and struct information
    file << "p_callback funclist[P_RULE_COUNT];\n";
    file << "typedef enum P_PARSER_ACTIONS {\n\tERROR,\n\tSHIFT,\n\tGOTO,\n\tREDUCE,\n\tACCEPT,\n\tCONFLICT\n} P_PARSER_ACTIONS;\n";
    file << "typedef struct P_TABLE_DATA {\n\tint name;\n\tint state;\n\tint funcindex;\n\tint size;\n} P_TABLE_DATA;\n";
    file << "typedef struct P_ARRAY {\n\tint length;\n\tunsigned int i_size;\n\tvoid *ptr;\n} P_ARRAY;\n";
    file << "P_TABLE_DATA **table;\n\n";
    // add function signatures
    file << "void INIT_TABLE();\n";
    file << "P_TABLE_DATA ACTION(int state, P_ELEMENTS tok);\n";
    file << "void PARSE();\n";
    // conclude and close file
    file << "#endif";
    file.close();
    // create and open parser output file
    file.open("out/p_output_parser.c");
    // add dependencies, libraries, and header files
    file << "#include \"p_output_scanner.h\"\n";
    file << "#include \"p_output_parser.h\"\n";
    // create function to initialize table
    file << "void INIT_TABLE() {\n";
    file << "\ttable = malloc(P_STATE_COUNT * sizeof(P_TABLE_DATA*));\n";
    file << "\tfor (int i = 0; i < P_STATE_COUNT; ++i) {\n";
    file << "\t\ttable[i] = malloc(P_ELEMENT_COUNT * sizeof(P_TABLE_DATA));\n";
    file << "\t\tmemset(table[i], 0, sizeof(P_TABLE_DATA));\n\t}\n\n";
    for (auto x : _hf->getTransitions()) {
        auto a = x.first.first;
        auto b = x.first.second;
        auto c = x.second;
        if (b.at(0) == '#') {
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].name = SHIFT;\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].state = " << c << ";\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].funcindex = -1;\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].size = 0;\n\n";
        }
        else {
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].name = GOTO;\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].state = " << c << ";\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].funcindex = -1;\n";
            file << "\ttable[" << a << "][" << _elementtoks[b] << "].size = 0;\n\n";
        }
    }

    auto states = _hf->getStates();
    auto transitions = _hf->getTransitions();
    auto follow_set = _hf->getFollowSet();

    for (int i = 0; i < _hf->getStateCount(); ++i) {
        for (int j = 0; j < states[i].size(); ++j) {
            auto handle = states[i][j];
            if (handle.closed()) {
                auto rule = handle.getRule();
                auto rule_lhs = rule->getLeft();
                auto lhs_name = rule_lhs->getName();
                auto reduce_triggers = follow_set[lhs_name + "@" + std::to_string(i)];

                if (lhs_name == "S*") {
                    file << "\ttable[" << i << "][" << _elementtoks["$"] << "].name = ACCEPT;\n";
                    file << "\ttable[" << i << "][" << _elementtoks["$"] << "].state = 0;\n";
                    file << "\ttable[" << i << "][" << _elementtoks["$"] << "].funcindex = -1;\n";
                    file << "\ttable[" << i << "][" << _elementtoks["$"] << "].size = 0;\n\n";
                    continue;
                }
                
                std::set<std::string> found;
                for (auto follow : follow_set) {
                    auto head = follow.first;
                    auto tagloc = std::find(head.rbegin(), head.rend(), '@');
                    auto index = std::distance(tagloc, head.rend()) -1;
                    auto content = head.substr(0, index);

                    if (content == lhs_name) {
                        for (auto dest : follow.second) {
                            if (dest == "$") 
                                content = dest;
                            else {
                                tagloc = std::find(dest.rbegin(), dest.rend(), '@');
                                index = std::distance(tagloc, dest.rend()) -1;
                                content = "#" + dest.substr(1, index-2);
                            }

                            if (found.find(content) == found.end()) 
                                found.insert(content); else continue;

                            file << "\ttable[" << i << "][" << _elementtoks[content] << "].name = REDUCE;\n";
                            file << "\ttable[" << i << "][" << _elementtoks[content] << "].state = " << transitions[HandleDictPair(i, lhs_name)] << ";\n";
                            file << "\ttable[" << i << "][" << _elementtoks[content] << "].funcindex = " << j << ";\n";
                            file << "\ttable[" << i << "][" << _elementtoks[content] << "].size = " << rule->getRight()->getChildren().size() << ";\n\n";
                        }
                    }
                }
            }
        }
    }   
    file << "}\n\n";
    // add definition for ACTION function, which uses table
    file << "P_TABLE_DATA ACTION (int state, P_ELEMENTS tok) {\n";
    file << "\treturn table[state][tok];\n";
    file << "}\n\n";
    // add definition for PARSE function, which takes no arguments
    file << "void PARSE () {\n";
    file << "\tP_LIST_TYPE states = P_LIST_INIT(sizeof(int));\n";
    file << "\tP_LIST_TYPE values = P_LIST_INIT(sizeof(P_PARSE_CLASS));\n\n";
    file << "\tP_ELEMENTS tok = LEX();\n";
    file << "\tint startstate = P_START_LIT;\n";
    file << "\tP_LIST_PUSH(&states, &startstate);\n\n";
    file << "\twhile (1) {\n";
    file << "\t\tint *current = (int*)P_LIST_TOP(&states);\n";
    file << "\t\tP_TABLE_DATA act = ACTION(*current, tok);\n";
    file << "\t}\n";
    file << "}\n\n";
    // close output file
    file.close();
}