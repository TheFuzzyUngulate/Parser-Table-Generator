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

void run_error(const char* ch) {
    std::cerr << ch << std::endl;
    exit(-1);
}

int main(int argc, char **argv) {

    vector<string> arguments(argv, argv+argc);
    struct flags flags = handle_args(arguments, 4);

    std::ifstream myfile;
    std::ofstream scfile, prfile;
    myfile.open(flags.input_file);
    if (!myfile.is_open()) {
        run_error("unable to open file");
    }

    Scanner *sc = new Scanner((std::fstream*)&myfile);
    Parser *par = new Parser(sc, flags);
    par->parse();
    if (flags.PRINT_PARSE_TREE)
        par->print_root();

    auto root = par->getroot();
    ASTProcessor proc = ASTProcessor(root);
    auto res = proc.process_ast_lalr1(sc->getstartstate());
    if (flags.PRINT_RULES) {
        for (auto step : res)
            step->print();
    }
    
    // new handlefinder stuff
    HandleFinder hfind = HandleFinder(res, proc.get_alphabet(), sc->getstartstate());
    hfind.exec();
    if (flags.PRINT_GRAMMAR)
        hfind.print_alt_grammar();

    CodeGenerator cgen = CodeGenerator(&hfind);
    cgen.genPrereqs();
    cgen.genParserFiles();
    cgen.genScannerFiles();

    return EXIT_SUCCESS;
}