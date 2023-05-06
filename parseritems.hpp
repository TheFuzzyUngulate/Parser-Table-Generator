#ifndef PARSERITEMS_HPP
#define PARSERITEMS_HPP
#pragma once

using std::cout, std::cerr, std::string, std::vector, std::make_pair;

class ParserItem {
    public:
        ParserItem(int n, string t) {
            _name = n;
            _type = t;
        }

        int name() {return _name;}
        string type() {return _type;}

    protected:
        int _name;
        string _type;
};

class ParserRule : public ParserItem {
    public:
        ParserRule(int n) : ParserItem(n, "nonterm") {}
};

class ParserTok : public ParserItem {
    public:
        ParserTok(int n) : ParserItem(n, "terminal") {}
};

class ParserRed : public ParserItem {
    public:
        ParserRed(int n, int c) : ParserItem(n, "reduce") {arg_c = c;}
        int getarg_count() {return arg_c;}
    private:
        int arg_c;
};

typedef std::pair<Tokens, Tokens> ParsePair;
typedef std::map<ParsePair, vector<Tokens>*> ParseDict;

#endif