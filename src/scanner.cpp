#include "../include/scanner/scanner.hpp"

void Scanner::scan_warn(const char *ch) {
    cerr << "scanner: " << ch << " at line " << lineno << std::endl;
}

void Scanner::scan_err(const char *ch) {
    scan_warn(ch);
    exit(-1);
}

int Scanner::get() {
    if (!unget_list.empty()) {
        char ch = unget_list.back();
        unget_list.pop_back();
        return ch;
    }

    if (file->eof()) return 0;
    else {
        char res = 0;
        file->read(&res, 1);
        return res;       
    }
}

void Scanner::unget(int ch) {
    unget_list.push_back(ch);
}

Tokens Scanner::lex() {
    while(1) {
        char ch;
        //lexeme.clear();

        do ch = get();
        while (ch != 0 && isspace(ch) && ch != '\n');

        // directives
        if (ch == '%') {
            string str;
            while ((ch = get()) == '%' || isalpha(ch)) str += ch; 
            unget(ch);

            if (str == "%") {
                state++;
                return Tokens::S_DELIM;
            }
            else
            if (str == "start") {
                if (start_state != "")
                    scan_err("multiple start states specified");
                do ch = get();
                while (ch != 0 && isspace(ch) && ch != '\n');
                string ddr;
                while (isalpha(ch)) {
                    ddr += ch;
                    ch = get();
                } unget(ch);
                start_state = ddr;
            }
            else scan_err("illegal symbol");
        }

        if (state == 0) {
            switch(ch) {
                case 0:
                    return Tokens::ENDFILE;

                case ':':
                    ch = get();
                    if (ch == '=') return Tokens::S_TRANSIT;
                    else scan_err("invalid token \':\'");
                    break;
                
                /*case '%':
                    ch = get();
                    if (ch == '%') {
                        state = 1;
                        return Tokens::S_DELIM;
                    } else scan_err("illegal symbol");
                    break;*/

                case '\n':
                    lineno++;
                    return Tokens::S_NEWLINE;
                
                case '\"':
                    lexeme.clear();
                    do {
                        ch = get();
                        lexeme += ch;
                    } while (isascii(ch) && ch != '\"');
                    lexeme.pop_back();
                    if (ch == '\"') return Tokens::S_CONTENT;
                    else scan_err("open quotation mark");
                    break;

                default:
                    lexeme.clear();
                    if (isalpha(ch)) {
                        lexeme += ch;
                        
                        do {
                            ch = get();
                            lexeme += ch;
                        } while (isalnum(ch));
                        
                        lexeme.pop_back();
                        unget(ch);
                        return Tokens::S_STRING;
                    } else {
                        string err = "invalid char ";
                        err += ch;
                        scan_err(err.c_str());
                    }
            }
        }
        else
        if (state == 1) {
            switch(ch) {
                case 0:
                    if (!reached_end) {
                        reached_end = true;
                        unget(ch);
                        return Tokens::BREAK;
                    } else return Tokens::ENDFILE;

                case '[': return Tokens::LOPT;
                case ']': return Tokens::ROPT;
                case '{': return Tokens::LREP;
                case '}': return Tokens::RREP;
                case '|': return Tokens::BAR;
                case '=':
                    ch = get();
                    if (ch == '>') return Tokens::ARROW;
                    else scan_err("invalid token");
                    break;
                
                case '\n':
                    lineno++;
                    return Tokens::BREAK;
                
                /*case '\"':
                    lexeme.clear();
                    do {
                        ch = get();
                        lexeme += ch;
                    } while (isascii(ch) && ch != '\"');
                    lexeme.pop_back();
                    if (ch == '\"') return Tokens::TOK;
                    else scan_err("open quotation mark");
                    break;*/

                default:
                    lexeme.clear();
                    if (isalpha(ch)) {
                        lexeme += ch;
                        
                        do {
                            ch = get();
                            lexeme += ch;
                        } while (isalnum(ch));
                        
                        lexeme.pop_back();
                        unget(ch);
                        
                        if (lexeme == "empty") return Tokens::EMPTY;
                        else return Tokens::RULE;
                    } else {
                        string err = "invalid char ";
                        err += ch;
                        scan_err(err.c_str());
                    }
            }
        }
    }
}