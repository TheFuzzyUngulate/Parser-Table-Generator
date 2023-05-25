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

#include "scanner.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "parseritems.hpp"
#include "runutils.hpp"
#include "astprocessing.hpp"
#include "asthandles.hpp"

int main(int argc, char **argv) {

    vector<string> arguments(argv, argv+argc);
    string filename = handle_args(arguments, 4);

    std::ifstream myfile;
    myfile.open(filename);
    if (!myfile.is_open()) {
        run_error("unable to open file");
    }

    Scanner *sc = new Scanner((std::fstream*)&myfile);
    Parser *par = new Parser(sc);
    par->parse();
    if (flags.PRINT_PARSE_TREE)
        par->print_root();

    auto root = par->getroot();
    auto res = process_ast_lalr1(root);
    
    // new handlefinder stuff
    HandleFinder hfind = HandleFinder(res);
    hfind.exec();
    hfind.alt_grammar();

    return EXIT_SUCCESS;
}