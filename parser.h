#pragma once
#include "tree.h"

bool Parser();
bool signal_program(Node* Tree);
bool program(Node* root);
bool procedure_identifier(Node* root);
bool identifier(Node* root);
bool block(Node* root);
bool declarations(Node* root);
bool var_decl_list(Node* root);
bool var_decl(Node* root);
bool statements_list(Node* root);
bool label_declarations(Node* root);
bool unsigned_integer(Node* root);
bool labels_list(Node* root);
bool statement(Node* root);
bool condition_statement(Node* root);
bool incomplete_condition_statement(Node* root);
bool alternative_part(Node* root);
bool conditional_expression(Node* root);
bool variable_identifier(Node* root);

struct Parser_error {
    int type;
    int row;
    int column;
    string str;
};