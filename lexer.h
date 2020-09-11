#pragma once

#include <algorithm> 
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <iomanip>
using namespace std;

extern int const_begin_value;
extern int identif_begin_value;

struct TSymbol {
    char value;
    int attr;
};

extern TSymbol symbol;

struct Lexem {
    string value;
    int code;
    int row;
    int column;
};

extern Lexem lexem;

struct Lexer_error {
    int type;
    int row;
    int column;
    char sym;
};

extern Lexer_error lex_error;

// void generate_symbol_cat(int *attributes);

void lexem_table_print(deque<Lexem> lexem_table);

void const_table_print(map<int, int> const_table);

void identif_table_print(map<string, int> identif_table);

// void show_error(Lexer_error lex_error);

int key_table_search(string word);

// int const_table_search(int constant, map<int, int> &const_table);

int const_table_search_code(int code, map<int, int> &const_table);

// int const_table_form(int constant, map<int, int> &const_table);

// int identif_table_search(string word, map<string, int> &identif_table);

string identif_table_search_code(int code, map<string, int> &identif_table);

// int identif_table_form(string word, map<string, int> &identif_table);

// TSymbol get_sym(ifstream &fin);

void Lexer(string path, deque<Lexem> &lexem_table, map<int, int> &const_table,
           map<string, int> &identif_table);
