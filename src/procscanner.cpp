#include "../include/regex/procscanner.hpp"

void RegExScanner::scan_warn(const char *ch) {
    cerr << "scanner: " << ch << " at line " << lineno << std::endl;
}

void RegExScanner::scan_err(const char *ch) {
    scan_warn(ch);
    exit(-1);
}

int RegExScanner::get() {
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

void RegExScanner::unget(int ch) {
    unget_list.push_back(ch);
}

int RegExScanner::lex() {
    while(1) {
        char ch;
        //lexeme.clear();

        do ch = get(); 
        while (ch != 0 && isspace(ch) && ch != '\n');

        switch(ch) {
            case 0:
                scan_err("reached EOF without parser rules");

            case ':':
                ch = get();
                if (ch == '=') return RegExTokens::PS_TRANSIT;
                else scan_err("invalid token");
                break;
            
            case '\n':
                lineno++;
                ch = get();
                if (ch == '%') {
                    char sh = get();
                    if (sh == '%') {
                        char th = get();
                        if (th == '\n') return RegExTokens::PS_ENDFILE;
                        unget(th);
                    } unget(sh);
                } unget(ch);
                return RegExTokens::PS_NEWLINE;
            
            case '\"':
                lexeme.clear();
                do {
                    ch = get();
                    lexeme += ch;
                } while (isascii(ch) && ch != '\"');
                lexeme.pop_back();
                if (ch == '\"') return RegExTokens::PS_CONTENT;
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
                    return RegExTokens::PS_STRING;
                } else scan_err("invalid token");
        }
    }
}