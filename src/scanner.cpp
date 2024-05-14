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

void Scanner::unlex(Tokens tok) {
    unlex_list.push_back(tok);
}

void Scanner::prescan()
{
    int ln = 1;
    string line;

    for (; std::getline(*file, line); ++ln) 
    {
        if (line.size() > 1 && line[0] == '%') 
        {    
            char ch;
            string str;

            // remove first char
            line.erase(line.begin());
            
            while (!line.empty()) {
                ch = line[0];
                line.erase(line.begin());
                if (ch != '%' && !isalpha(ch)) break;
                str.push_back(ch);
            } line.insert(line.begin(), ch);

            if (str == "%") {
                dirs.trs.push_back({ln, true});
                continue;
            }
            else
            {
                dirs.trs.push_back({ln, false});
                
                if (str == "start") 
                {
                    // if start already set, this is an error
                    if (dirs.start != "")
                        scan_err("multiple start states specified");

                    
                    // eliminate spaces before the next argument
                    while (!line.empty()) {
                        ch = line[0];
                        line.erase(line.begin());
                        if (ch == 0 || !isspace(ch) || ch == '\n') break;
                    }

                    // we need this.. again...
                    str.clear();

                    // get next string
                    if (isalpha(ch)) {
                        str.push_back(ch);
                        while (!line.empty()) {
                            if (!isalpha(ch)) break;
                            ch = line[0];
                            line.erase(line.begin());
                            str.push_back(ch);
                        }
                    } else scan_err("invalid start state argument provided");

                    dirs.start = str;
                }
                else
                if (str == "ignore")
                {
                    // eliminate spaces before the next argument
                    while (!line.empty()) {
                        ch = line[0];
                        line.erase(line.begin());
                        if (ch == 0 || !isspace(ch) || ch == '\n') break;
                    }

                    str.clear();
                    
                    str.push_back(ch);
                    while (!line.empty()) {
                        if (isspace(ch) && ch != ' ') break;
                        ch = line[0];
                        line.erase(line.begin());
                        str.push_back(ch);
                    }

                    if (str == "\\n") {
                        dirs.ignored.insert('\n');
                    } else if (str == "\\r") {
                        dirs.ignored.insert('\r');
                    } else if (str == "\\w") {
                        dirs.ignored.insert(' ');
                    } else if (str == "\\t") {
                        dirs.ignored.insert('\t');
                    } else {
                        if (str.size() == 1) {
                            dirs.ignored.insert(str[0]);
                        } else {
                            cout << "suggested " << str << std::endl;
                            scan_err("invalid ignore char suggested.\n");
                        }
                    }
                }
                else
                if (str == "ateof")
                {
                    if (!dirs.ateof.empty()) {
                        scan_warn("only one ateof character may be set");
                    }

                    // eliminate spaces before the next argument
                    while (!line.empty()) {
                        ch = line[0];
                        line.erase(line.begin());
                        if (ch == 0 || !isspace(ch) || ch == '\n') break;
                    }

                    str.clear();
                    
                    str.push_back(ch);
                    while (!line.empty()) {
                        if (isspace(ch) && ch != ' ') break;
                        ch = line[0];
                        line.erase(line.begin());
                        str.push_back(ch);
                    }

                    dirs.ateof = str;
                }
                else
                if (str == "nodecollapse")
                {
                    dirs.nodecollapse = true;
                }
            }
        }
    }

    file->clear();
    file->seekg(0);
}

Tokens Scanner::lex()
{
    while (1) 
    {
        char ch;

        if (!unlex_list.empty()) {
            auto tok = unlex_list.back();
            unlex_list.pop_back();
            return tok;
        }

        do ch = get();
        while (ch != 0 && isspace(ch) && ch != '\n');

        if (dirs.state == 0)
        {
            switch(ch) 
            {
                case 0:
                    return Tokens::ENDFILE;

                case '{':
                    return Tokens::LOPT;

                case '}':
                    return Tokens::ROPT;

                case '\n':
                    lineno++;
                    statetrans();
                    //return Tokens::NEWLINE;
                    break;

                default:
                    lexeme.clear();
                    
                    if (isascii(ch) && !isspace(ch)) {
                        do {
                            if (ch == '\\') {
                                ch = get();
                                if (ch == 'w') {
                                    lexeme += ' ';
                                    ch = get();
                                } else if (ch == '{') {
                                    lexeme += '{';
                                    ch = get();
                                } else if (ch == '}') {
                                    lexeme += '}';
                                    ch = get();
                                } else {
                                    lexeme += '\\';
                                    lexeme += ch;
                                    ch = get();
                                }
                            }
                            else {
                                lexeme += ch;
                                ch = get();
                            }
                        } while (isascii(ch) && !isspace(ch) && ch != '{' && ch != '}');
                        unget(ch);
                        return Tokens::STRING;
                    } else {
                        string err = "invalid char ";
                        err += ch;
                        scan_err(err.c_str());
                    }
            }
        }
        else
        if (dirs.state == 1) 
        {
            switch(ch) {
                case 0:
                    /**if (!reached_end) {
                        reached_end = true;
                        unget(ch);
                        return Tokens::BREAK;
                    } else**/ return Tokens::ENDFILE;

                case '[': return Tokens::LOPT;
                case ']': return Tokens::ROPT;
                case '{': return Tokens::LREP;
                case '}': return Tokens::RREP;
                case '|': return Tokens::BAR;
                case ';': return Tokens::BREAK;

                case ':':
                    ch = get();
                    if (ch == ':') {
                        ch = get();
                        if (ch == '=')
                            return Tokens::ARROW;
                        else scan_err("invalid token");
                    } else scan_err("invalid token");
                    break;

                case '=':
                    ch = get();
                    if (ch == '>') return Tokens::ARROW;
                    else scan_err("invalid token");
                    break;
                
                case '\n':
                    lineno++;
                    statetrans();
                    break;
                
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
                        } while (isalnum(ch) || ch == '_');
                        
                        lexeme.pop_back();
                        unget(ch);
                        
                        if (lexeme == "empty") return Tokens::EMPTY;
                        else return Tokens::LIT;
                    } else {
                        string err = "invalid char ";
                        err += ch;
                        scan_err(err.c_str());
                    }
            }
        }
    }
}