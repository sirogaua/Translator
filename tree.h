#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

struct Node {
    string data;
    vector<Node *> leafs;
    
    Node *add_leaf(string data) { 
        Node *leaf = new struct Node;
        leaf->data = data;
        leafs.push_back(leaf); 
        return leaf;
    }
};

Node *new_leaf(string data);

void print_tree(Node *root, string path);