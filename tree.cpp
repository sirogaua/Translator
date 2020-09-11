#include "tree.h"

Node *new_leaf(string data) {
    Node *leaf = new struct Node;
    leaf->data = data;
    return leaf;
}

void printNT(const string prefix, struct Node *node, bool last, ofstream &fout) {
    if (node == nullptr) return;
    int leaf_size = node->leafs.size();
    cout << prefix;
    fout << prefix;

    cout << (last ? "├──" : "└──");
    fout << (last ? "├──" : "└──");

    cout << node->data << endl;
    fout << node->data << endl;
    
    for (int i = 0; i < leaf_size; i++) {
        if (i == leaf_size - 1) {
            printNT(prefix + (last ? "│   " : "    "), node->leafs[i], false, fout);
        } else {
            printNT(prefix + (last ? "│   " : "    "), node->leafs[i], true, fout);
        }
    }
}

void print_tree(struct Node *node, string path) { 
    cout << "\t\t\tTREE\n" << endl;
    ofstream fout(path.replace(path.find("input.sig"), 9, string("generated.txt")));
    printNT("", node, false, fout); 
    cout << "\n\n";
}
