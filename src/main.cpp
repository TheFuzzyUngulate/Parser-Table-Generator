// #include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>
#include <deque>

#include "../include/scanner/scanner.hpp"
#include "../include/parser/parser.hpp"
#include "../include/parser/parseritems.hpp"
#include "../include/ast/ast.hpp"
#include "../include/ast/astprocessing.hpp"
#include "../include/ast/asthandles.hpp"
#include "../include/utilities/runutils.hpp"
#include "../include/utilities/general_utils.hpp"
#include "../include/gen/codegen.hpp"
#include "../include/regex/regprocess.hpp"

void run_error(const char* ch) {
    std::cerr << ch << std::endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    vector<string> arguments(argv, argv+argc);
    struct flags flags = handle_args(arguments);

    std::ifstream myfile;
    std::ofstream scfile, prfile;
    myfile.open(flags.input_file);
    if (!myfile.is_open()) {
        run_error("unable to open file");
    }

    Scanner *sc = new Scanner((std::fstream*)&myfile);
    Parser  *pr = new Parser(sc, flags);
    pr->parse();

    if (flags.PRINT_PARSE_TREE) {
        for (auto k : pr->psitems())
            k->print();
    }
    
    ASTProcessor proc = ASTProcessor(pr->psitems(), flags.PRINT_RULE_GENERATION);
    auto res = proc.process_ast_lalr1(sc->sdir().start);
    if (flags.PRINT_RULES) {
        for (int i = 0; i < res.size(); ++i) {
            auto step = res[i];
            printf("step %i:\n", i);
            step->print(1);
        }
    }

    // get alphabet
    auto alphabet = proc.get_alphabet();

    // add all tokens, even unused ones
    for (auto tok : pr->scitems()) {
        alphabet.insert("#" + tok.name);
    }

    // new handlefinder stuff
    HandleFinder hfind = HandleFinder(res, alphabet, sc->sdir().start);
    hfind.exec();
    if (flags.PRINT_GRAMMAR) {
        hfind.print_states();
        hfind.print_transitions();
        hfind.print_alt_grammar();
        hfind.print_first_and_follow_sets();
    }

    // add S* and $ to alphabet due to handle finder
    alphabet.insert("$");
    alphabet.insert("S*");

    // code generation
    if (flags.PRODUCE_GENERATOR) 
    {
        // change regexes to tuple list    
        deque<pair<reglit, string>> regexes;
        auto fake = pr->scitems();
        for (auto item : fake) {
            regexes.push_back({item, re_conv(item.regstr, 0)});
        }
        
        CodeGenerator cgen = CodeGenerator(&hfind, alphabet, sc->sdir(), res, regexes, flags.output_file);
        cgen.generate();
    }

    return EXIT_SUCCESS;
}