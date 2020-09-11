#include "parser.h"

#include "lexer.h"

using namespace std;

deque<Lexem> lexem_table;
map<int, int> const_table;
map<string, int> identif_table;

Node* Tree = new struct Node;

Parser_error parser_error;

void show_error(Parser_error parser_error) {
    printf("Parser: Error (row %d, column %d): ", parser_error.row,
           parser_error.column);
    if (parser_error.type == 0)
        cout << "unexpected end of file\n" << endl;
    else if (parser_error.type == 1)
        cout << "identifier expected\n" << endl;
    else if (parser_error.type == 2)
        cout << "statment expected\n" << endl;
    else if (parser_error.type == 3)
        cout << "symbol '" << parser_error.str << "' expected\n" << endl;
    else if (parser_error.type == 4)
        cout << "keyword '" << parser_error.str << "' expected\n" << endl;
    else if (parser_error.type == 5)
        cout << "constant expected\n" << endl;
    else if (parser_error.type == 6)
        cout << "unexpected tokens after end of program\n" << endl;
}

bool Parser() {
    if (!signal_program(Tree)) {
        show_error(parser_error);
        return false;
    }
    return true;
}

bool signal_program(Node* root) {
    root->data = "<signal program>";
    return program(root);
}

bool program(Node* root) {
    Node* leaf = root->add_leaf("<program>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("PROGRAM")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " PROGRAM");
    } else {
        parser_error = (Parser_error){.type = 4,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "PROGRAM"};
        return false;
    }
    if (!procedure_identifier(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == 59) {  // ;
        lexem_table.pop_front();
        leaf->add_leaf("59 ;");
    } else {
        parser_error = (Parser_error){.type = 3,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ";"};
        return false;
    }
    if (!block(leaf)) return false;
    if (lexem_table.empty()) {
        // parser_error =
        //     (Parser_error){.type = 3, .row = 0, .column = 0, .str = "."};
        return false;
    }
    if (lexem_table.front().code == 46) {  // .
        lexem_table.pop_front();
        leaf->add_leaf("46 .");
    } else {
        parser_error = (Parser_error){.type = 3,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "."};
        return false;
    }
    if (!lexem_table.empty()) {
        parser_error = (Parser_error){.type = 6,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    return true;
}

bool procedure_identifier(Node* root) {
    Node* leaf = root->add_leaf("<procedure-identifier>");
    return identifier(leaf);
}

bool identifier(Node* root) {
    Node* leaf = root->add_leaf("<identifier>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code / identif_begin_value == 1) {
        string identif_name =
            identif_table_search_code(lexem_table.front().code, identif_table);
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " " + identif_name);
        return true;
    } else {
        parser_error = (Parser_error){.type = 1,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
}

bool block(Node* root) {
    Node* leaf = root->add_leaf("<block>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (!declarations(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("BEGIN")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " BEGIN");
    } else {
        parser_error = (Parser_error){.type = 4,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "BEGIN"};
        return false;
    }
    if (!statements_list(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("END")) {
        string token_code = to_string(lexem_table.front().code);
        parser_error = (Parser_error){.type = 3,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column + 3,
                                      .str = "."};
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " END");
    } else {
        return false;
    }
    return true;
}

bool declarations(Node* root) {
    Node* leaf = root->add_leaf("<declarations>");
    if (lexem_table.front().code == key_table_search("VAR")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " VAR");
        return var_decl_list(leaf);
    }
    return label_declarations(leaf);
}

bool var_decl_list(Node* root) {
    Node* leaf = root->add_leaf("<var-decl-list>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("COND") ||
        lexem_table.front().code == key_table_search("STATE")) {
            if (!var_decl(leaf)) return false;
            if (lexem_table.empty()) {
                parser_error = (Parser_error){.type = 0,
                                            .row = lexem_table.front().row,
                                            .column = lexem_table.front().column,
                                            .str = ""};
                return false;
            }
            lexem_table.pop_front();
            leaf->add_leaf("62 >");
            if (!var_decl_list(leaf)) return false;
    } else {
        leaf->add_leaf("<empty>");
        return true;
    }
    return true;
}

bool var_decl(Node* root) {
    Node* leaf = root->add_leaf("<var-decl>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("COND")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " COND");
        if (!conditional_expression(leaf)) return false;
    } else if (lexem_table.front().code == key_table_search("STATE")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " STATE");
        if (!statement(leaf)) return false;
    } else {
        leaf->add_leaf("<empty>");
        return true;
    }
    return true;
}

bool label_declarations(Node* root) {
    Node* leaf = root->add_leaf("<label-declarations>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("LABEL")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " LABEL");
    } else {
        leaf->add_leaf("<empty>");
        return true;
    }
    if (!unsigned_integer(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (!labels_list(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == 59) {  // ;
        lexem_table.pop_front();
        leaf->add_leaf("59 ;");
    } else {
        parser_error = (Parser_error){.type = 3,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ";"};
        return false;
    }
    return true;
}

bool unsigned_integer(Node* root) {
    Node* leaf = root->add_leaf("<unsigned-integer>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code / const_begin_value == 1) {
        int const_val =
            const_table_search_code(lexem_table.front().code, const_table);
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " '" + to_string(const_val) + "'");
        return true;

    } else {
        parser_error = (Parser_error){.type = 5,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    return true;
}

bool labels_list(Node* root) {
    Node* leaf = root->add_leaf("<labels-list>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code != 44) {
        leaf->add_leaf("<empty>");
        return true;
    }
    if (lexem_table.front().code == 44) {  // ,
        lexem_table.pop_front();
        leaf->add_leaf("44 ,");
        if (!unsigned_integer(leaf)) return false;
        if (lexem_table.empty()) {
            parser_error = (Parser_error){.type = 0,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ""};
            return false;
        }
        if (!labels_list(leaf)) return false;
    }
    return true;
}

bool statements_list(Node* root) {
    Node* leaf = root->add_leaf("<statements-list>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code / const_begin_value != 1 &&
        lexem_table.front().code != key_table_search("IF") &&
        lexem_table.front().code != key_table_search("GOTO") &&
        lexem_table.front().code != 59) {
        parser_error = (Parser_error){.type = 2,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        leaf->add_leaf("<empty>");
        return true;
    }
    if (!statement(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (!statements_list(leaf)) return false;
    return true;
}

bool statement(Node* root) {
    Node* leaf = root->add_leaf("<statement>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code / const_begin_value == 1) {
        int const_val =
            const_table_search_code(lexem_table.front().code, const_table);
        if (!unsigned_integer(leaf)) return false;
        if (lexem_table.empty()) {
            parser_error = (Parser_error){.type = 0,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ""};
            return false;
        }
        if (lexem_table.front().code == 58) {  // :
            lexem_table.pop_front();
            leaf->add_leaf("58 :");
        } else {
            parser_error = (Parser_error){.type = 3,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ":"};
            return false;
        }
        if (!statement(leaf)) return false;
        if (lexem_table.empty()) {
            parser_error = (Parser_error){.type = 0,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ""};
            return false;
        }
        return true;
    } else if (lexem_table.front().code == key_table_search("GOTO")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " GOTO");
        if (!unsigned_integer(leaf)) return false;
        if (lexem_table.front().code == 59) {  // ;
            lexem_table.pop_front();
            leaf->add_leaf("59 ;");
        } else {
            parser_error = (Parser_error){.type = 3,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ";"};
            return false;
        }
        return true;
    } else if (lexem_table.front().code == key_table_search("IF")) {
        if (!condition_statement(leaf)) return false;
        if (lexem_table.empty()) {
            parser_error = (Parser_error){.type = 0,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ""};
            return false;
        }
        if (lexem_table.front().code == key_table_search("ENDIF")) {
            string token_code = to_string(lexem_table.front().code);
            lexem_table.pop_front();
            leaf->add_leaf(token_code + " ENDIF");
        } else {
            parser_error = (Parser_error){.type = 4,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = "ENDIF"};
            return false;
        }
        if (lexem_table.empty()) {
            parser_error = (Parser_error){.type = 0,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ""};
            return false;
        }
        if (lexem_table.front().code == 59) {  // ;
            lexem_table.pop_front();
            leaf->add_leaf("59 ;");
        } else {
            parser_error = (Parser_error){.type = 3,
                                          .row = lexem_table.front().row,
                                          .column = lexem_table.front().column,
                                          .str = ";"};
            return false;
        }
        return true;
    } else if (lexem_table.front().code == 59) {
        lexem_table.pop_front();
        leaf->add_leaf("59 ;");
        return true;
    }
    parser_error = (Parser_error){.type = 2,
                                  .row = lexem_table.front().row,
                                  .column = lexem_table.front().column,
                                  .str = ""};
    return false;
}

bool condition_statement(Node* root) {
    Node* leaf = root->add_leaf("<condition-statement>");
    if (!incomplete_condition_statement(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (!alternative_part(leaf)) return false;
    return true;
}

bool incomplete_condition_statement(Node* root) {
    Node* leaf = root->add_leaf("<incomplete-condition-statement>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("IF")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " IF");
    } else {
        parser_error = (Parser_error){.type = 4,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "IF"};
        return false;
    }
    if (!conditional_expression(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("THEN")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " THEN");
    } else {
        parser_error = (Parser_error){.type = 4,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "THEN"};
        return false;
    }
    if (!statements_list(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    return true;
}

bool conditional_expression(Node* root) {
    Node* leaf = root->add_leaf("<conditional-expression>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (!variable_identifier(leaf)) return false;
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == 61) {  // =
        lexem_table.pop_front();
        leaf->add_leaf("61 =");
    } else {
        parser_error = (Parser_error){.type = 3,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = "="};
        return false;
    }
    if (!unsigned_integer(leaf)) return false;
    return true;
}

bool variable_identifier(Node* root) {
    Node* leaf = root->add_leaf("<variable-identifier");
    return identifier(leaf);
}

bool alternative_part(Node* root) {
    Node* leaf = root->add_leaf("<alternative-part>");
    if (lexem_table.empty()) {
        parser_error = (Parser_error){.type = 0,
                                      .row = lexem_table.front().row,
                                      .column = lexem_table.front().column,
                                      .str = ""};
        return false;
    }
    if (lexem_table.front().code == key_table_search("ELSE")) {
        string token_code = to_string(lexem_table.front().code);
        lexem_table.pop_front();
        leaf->add_leaf(token_code + " ELSE");
    } else {
        leaf->add_leaf("<empty>");
        return true;
    }
    if (!statements_list(leaf)) return false;
    return true;
}

int main(int argc, char* argv[]) {
    string path(argv[1]);
    path = "./" + path + "/input.sig";
    Lexer(path, lexem_table, const_table, identif_table);

    deque<Lexem> lexem_table_swap = lexem_table;

    Parser();
    print_tree(Tree, path);

    lexem_table_print(lexem_table_swap);
    // const_table_print(const_table);
    // identif_table_print(identif_table);

    return 0;
}