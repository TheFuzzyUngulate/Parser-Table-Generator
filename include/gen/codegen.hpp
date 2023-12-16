#ifndef CODEGEN_HPP
#define CODEGEN_HPP
#pragma once

#include <fstream>
#include <iostream>

#include "../ast/asthandles.hpp"

class CodeGenerator {
    public:
        CodeGenerator(HandleFinder *h) {
            _hf = h;
            _elements = h->get_all_terms_and_nterms();

            auto tokcount = 1;
            for (auto x : _elements) {
                if (x == "$")
                    _elementtoks[x] = "P_TOK_END";
                else
                if (x == "S*")
                    _elementtoks[x] = "P_START_LIT";
                else {
                    if (x.at(0) == '#') {
                        if (isalpha(x.substr(1)))
                            _elementtoks[x] = "P_TOK_" + x.substr(1);
                        else _elementtoks[x] = "P_TOK_" + std::to_string(tokcount++);
                    }
                    else {
                        int barcount = 0;
                        while (x.at((x.size()-1)-barcount) == '\'')
                            ++barcount;
                        auto true_x = x.substr(0, x.size()-barcount);
                        std::string ret = "P_LIT_" + true_x;
                        for (int i = 0; i < barcount; ++i)
                            ret += "_BAR";
                        _elementtoks[x] = ret;
                    }
                }
            }
        }

        void genPrereqs();
        void genScannerFiles();
        void genParserFiles();

    private:
        HandleFinder *_hf;
        std::vector<std::string> _elements;
        std::map<std::string, std::string> _elementtoks;
};

#endif