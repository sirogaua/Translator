#include "lexer.h"

using namespace std;

int const_begin_value = 501;
int identif_begin_value = 1001;
int attributes[256];
string buf;
bool suppres_output;

TSymbol symbol;

Lexem lexem;

Lexer_error lex_error;

map<string, int> keyword_table = {
    {"PROGRAM", 401}, {"BEGIN", 402}, {"END", 403},  {"GOTO", 404},
    {"LABEL", 405},   {"IF", 406},    {"THEN", 407}, {"ELSE", 408},
    {"ENDIF", 409},   {"VAR", 410},   {"COND", 411}, {"STATE", 412}};

void generate_symbol_cat(int *attributes) {
    // unprohibited
    int i;
    for (i = 0; i <= 255; ++i) attributes[i] = 6;

    // space symbols
    attributes[32] = 0;
    for (i = 8; i <= 13; i++) attributes[i] = 0;
    // digits
    for (i = 48; i <= 57; i++) attributes[i] = 1;

    // letters
    for (i = 65; i <= 90; i++) attributes[i] = 2;

    attributes[44] = 3;  // ,
    attributes[46] = 3;  // .
    attributes[58] = 3;  // :
    attributes[59] = 3;  // ;
    attributes[61] = 3;  // =
    attributes[62] = 3;  // >

    // multi-char delimiters for comments
    attributes[40] = 5;  // (
}

void lexem_table_print(deque<Lexem> lexem_table) {
    cout << "\t\t\tLEXEM TABLE\n" << endl;
    cout << left << "\t    " << setw(14) << "value" << setw(8) << "code"
         << setw(6) << "row" << setw(6) << "column  │\n"
         << "────────────────────────────────────────────────┤" << endl;
    for (Lexem lexem : lexem_table) {
        cout << left << "Output:    '" << setw(14) << lexem.value + "'"
             << setw(8) << lexem.code << setw(6) << lexem.row << setw(6)
             << lexem.column << "\t│" << endl;
    }
    cout << "\n\n";
}

void const_table_print(map<int, int> const_table) {
    cout << "\t\t\tCONSTANTS TABLE" << endl;
    for (auto it = const_table.begin(); it != const_table.end(); ++it) {
        cout << it->first << " : " << it->second << endl;
    }
    cout << "\n\n";
}

void identif_table_print(map<string, int> identif_table) {
    cout << "\t\t\tIDENTIFICATORS TABLE" << endl;
    for (auto it = identif_table.begin(); it != identif_table.end(); ++it) {
        cout << it->first << " : " << it->second << endl;
    }
    cout << "\n\n";
}

void show_error(Lexer_error lex_error) {
    printf("Lexer: Error (row %d, column %d): ", lex_error.row,
           lex_error.column);
    if (lex_error.type == 0)
        printf("empty file\n");
    else if (lex_error.type == 50)
        printf("unterminated comment\n");
    else if (lex_error.type == 51) {
        printf("'*' expected but ");
        switch (lex_error.sym) {
            case '\b':
                cout << "'\\b'"
                     << " found\n";
                break;
            case '\t':
                cout << "'\\t'"
                     << " found\n";
                break;
            case '\n':
                cout << "'\\n'"
                     << " found\n";
                break;
            case '\v':
                cout << "'\\v'"
                     << " found\n";
                break;
            case '\f':
                cout << "'\\f'"
                     << " found\n";
                break;
            case '\r':
                cout << "'\\r'"
                     << " found\n";
                break;
            default:
                printf("'%c' found\n", lex_error.sym);
                break;
        }
    } else if (lex_error.type == 60)
        printf("Illegal symbol: '%c'\n", lex_error.sym);
}

int key_table_search(string word) {
    map<string, int>::iterator it;
    it = keyword_table.find(word);
    if (it != keyword_table.end()) {
        return it->second;
    }
    return -1;
}

int const_table_search(int constant, map<int, int> &const_table) {
    map<int, int>::iterator it;
    it = const_table.find(constant);
    if (it != const_table.end()) {
        return it->second;
    }
    return -1;
}

int const_table_search_code(int code, map<int, int> &const_table) {
    map<int, int>::iterator it;
    for (it = const_table.begin(); it != const_table.end(); ++it)
        if (it->second == code) return it->first;
    return -1;
}

int const_table_form(int constant, map<int, int> &const_table) {
    int value = const_table.size() + const_begin_value;
    const_table.insert(make_pair(constant, value));
    return value;
}

int identif_table_search(string word, map<string, int> &identif_table) {
    map<string, int>::iterator it;
    it = identif_table.find(word);
    if (it != identif_table.end()) {
        return it->second;
    }
    return -1;
}

string identif_table_search_code(int code, map<string, int> &identif_table) {
    map<string, int>::iterator it;
    for (it = identif_table.begin(); it != identif_table.end(); ++it)
        if (it->second == code) return it->first;
    return "0empty";
}

int identif_table_form(string word, map<string, int> &identif_table) {
    int value = identif_table.size() + identif_begin_value;
    identif_table.insert(make_pair(word, value));
    return value;
}

TSymbol get_sym(ifstream &fin) {
    TSymbol temp_sym;
    fin.get(temp_sym.value);
    temp_sym.attr = attributes[int(temp_sym.value)];
    return temp_sym;
}

void Lexer(string path, deque<Lexem> &lexem_table, map<int, int> &const_table,
           map<string, int> &identif_table) {
    int constant, finded_constant, finded_keyword, finded_identif;
    int column_num = 1, row_num = 1;
    lex_error.type = 0;
    generate_symbol_cat(attributes);
    ifstream fin(path);
    ofstream fout(
        path.replace(path.find("input.sig"), 9, string("lexems.txt")));
    if (fin.peek() == ifstream::traits_type::eof()) {
        lex_error.type = 0;
        show_error(lex_error);
    }
    symbol = get_sym(fin);
    while (!fin.eof()) {
        buf = "";
        lexem.code = 0;
        suppres_output = false;
        switch (symbol.attr) {
            case 0:  // space symbols
                while (!fin.eof()) {
                    if (symbol.attr != 0) break;
                    if (symbol.value == int('\n')) {
                        row_num++;
                        column_num = 0;
                    }
                    symbol = get_sym(fin);
                    column_num++;
                }
                suppres_output = true;
                break;

            case 1:  // digits
                lexem.row = row_num;
                lexem.column = column_num;
                while (!fin.eof()) {
                    if (symbol.attr != 1) break;
                    buf += symbol.value;
                    symbol = get_sym(fin);
                    column_num++;
                }
                if (symbol.attr == 2) {
                    lex_error = (Lexer_error){.type = 60,
                                              .row = row_num,
                                              .column = column_num,
                                              .sym = symbol.value};
                    while (symbol.attr == 2) {
                        symbol = get_sym(fin);
                        column_num++;
                    }
                }
                constant = stoi(buf);
                finded_constant = const_table_search(constant, const_table);
                lexem.value = buf;
                if (finded_constant != -1) {
                    lexem.code = finded_constant;
                } else {
                    lexem.code = const_table_form(constant, const_table);
                }
                break;

            case 2:  // letters
                lexem.row = row_num;
                lexem.column = column_num;
                while (!fin.eof()) {
                    if (symbol.attr != 2 && symbol.attr != 1) break;
                    buf += symbol.value;
                    symbol = get_sym(fin);
                    column_num++;
                }
                finded_keyword = key_table_search(buf);
                lexem.value = buf;
                if (finded_keyword != -1) {
                    lexem.code = finded_keyword;
                } else {
                    finded_identif = identif_table_search(buf, identif_table);
                    if (finded_identif != -1) {
                        lexem.code = finded_identif;
                    } else {
                        lexem.code = identif_table_form(buf, identif_table);
                    }
                }
                break;

            case 3:  // single char delimiters
                lexem.row = row_num;
                lexem.column = column_num;
                lexem.code = int(symbol.value);
                // lexem.code = attributes[symbol.value];
                lexem.value = symbol.value;
                symbol = get_sym(fin);
                column_num++;
                break;

            case 5:
                if (fin.eof()) {
                    lexem.row = row_num;
                    lexem.column = column_num;
                    lexem.code = symbol.attr;
                    lexem.value = symbol.value;
                } else {
                    symbol = get_sym(fin);
                    lex_error.column = column_num;
                    lex_error.row = row_num;
                    column_num++;
                    if (symbol.value == int('*')) {
                        if (fin.eof()) {
                            lex_error.type = 50;
                        } else {
                            symbol = get_sym(fin);
                            column_num++;
                            do {
                                while (!fin.eof()) {
                                    if (symbol.value == int('*')) break;
                                    if (symbol.value == int('\n')) {
                                        row_num++;
                                        column_num = 0;
                                    }
                                    symbol = get_sym(fin);
                                    column_num++;
                                }
                                if (fin.eof()) {
                                    lex_error.type = 50;
                                    symbol.value = int('+');
                                    break;
                                } else {
                                    symbol = get_sym(fin);
                                    if (symbol.value == int('\n')) {
                                        row_num++;
                                        column_num = 0;
                                    }
                                    column_num++;
                                }
                            } while (symbol.value != int(')'));
                            if (symbol.value == int(')')) {
                                suppres_output = true;
                            }
                            if (!fin.eof()) {
                                symbol = get_sym(fin);
                                column_num++;
                            }
                        }
                    } else {
                        lex_error = (Lexer_error){.type = 51,
                                                  .row = row_num,
                                                  .column = column_num,
                                                  .sym = symbol.value};
                        if (symbol.value == int('\n')) {
                            row_num++;
                            column_num = 0;
                        }
                    }
                }
                break;

            case 6:  // unprohibited
                lex_error = (Lexer_error){.type = 60,
                                          .row = row_num,
                                          .column = column_num,
                                          .sym = symbol.value};
        }
        if (lex_error.type != 0) {
            show_error(lex_error);
            lex_error.type = 0;
            symbol = get_sym(fin);
            column_num++;
            suppres_output = true;
            // break;
        }
        if (!suppres_output) {
            lexem_table.push_back(lexem);
        }
    }
    for (Lexem lexem : lexem_table) {
        fout << lexem.code << " ";
    }

    fin.close();
    fout.close();
}