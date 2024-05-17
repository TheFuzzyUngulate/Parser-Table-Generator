#ifndef CODEGEN_HPP
#define CODEGEN_HPP
#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#include "../parser/parser.hpp"
#include "../ast/asthandles.hpp"
#include "../regex/regprocess.hpp"
#include "../utilities/directives.hpp"
#include "../utilities/general_utils.hpp"

typedef std::vector<std::pair<std::string, std::string>> regexlib;
typedef std::deque<std::deque<std::pair<int, Rule*>>> astsorting;

class CodeGenerator {
    public:
        CodeGenerator(HandleFinder *h, std::set<std::string> alphabet, s_dirs dirs, deque<Rule*> rules, deque<pair<reglit, string>> regexes, std::string filename) {
            _hf         = h;
            _fname      = filename;
            _rules      = rules;
            _regexes    = regexes;
            _elements   = alphabet;
            _directives = dirs;

            auto tokcount = 1;
            for (auto x : _elements) 
            {
                if (x == "$")
                    _elementtoks[x] = "P_TOK_END";
                else
                if (x == "S*")
                    _elementtoks[x] = "P_START_LIT";
                else {
                    if (x.at(0) == '#') {
                        if (islit(x))
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

        void generate();

    private:

        void ast_create_groups();
        void ast_create_groupnames();

        HandleFinder *_hf;
        std::string _fname;
        deque<Rule*> _rules;
        s_dirs _directives;
        deque<pair<reglit, string>> _regexes;
        astsorting _astgroups;
        std::deque<std::string> _astgroupnames;
        std::set<std::string> _elements;
        std::map<std::string, std::string> _elementtoks;
};

#endif