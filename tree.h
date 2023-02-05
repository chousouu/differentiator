#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double elem_t;

const int LEFT_NODE = 0x32;
const int RIGHT_NODE = 0x64;

struct Node 
{
    elem_t data;
    Node *left;
    Node *right;
    Node *parent;
    int type;
};

struct Tree
{
    Node *root;
};

enum TYPES
{
    TYPE_CONST = 0x001,
    TYPE_VAR   = 0x002,
    TYPE_OP    = 0x003,
    TYPE_UNARY = 0x004,
};

Tree *CreateTree(elem_t value);

Node *CreateNode(elem_t value, int type);

Node *AddNode(struct Node *tree, elem_t value, int child);

void InOrder(struct Node *tree);

void NodeDtor(struct Node *node);

void TreeDtor(struct Tree *tree);
#endif// TREE_H