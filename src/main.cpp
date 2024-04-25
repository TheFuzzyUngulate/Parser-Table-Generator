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
    struct flags flags = handle_args(arguments);

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
    if (flags.PRINT_GRAMMAR) {
        hfind.print_states();
        hfind.print_transitions();
        hfind.print_first_and_follow_sets();
    }

    // code generation
    if (flags.PRODUCE_GENERATOR) 
    {
        // change regexes to tuple list    
        regexlib regexes;
        auto fake = par->getregexes()->getChildren();
        for (auto item : fake) {
            if (item->getId() == "regex") {
                RegRule* re = (RegRule*)item;
                std::pair<std::string, std::string> pear = {re->getName(), re->getRegex()};
                regexes.push_back(pear);
            }
        }
        
        CodeGenerator cgen = CodeGenerator(&hfind, res, regexes, flags.output_file);
        cgen.generate();
    }

    return EXIT_SUCCESS;
}