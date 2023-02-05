#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "tree.h"

#define DEB(...) printf(__VA_ARGS__)

enum graphviz
{
    CREATE_NODE = 0,
    LINK_NODE   = 1,
};

enum ERRORS
{
    OK          = 0x000,
    ERROR_ALNUM = 0x001,
    ERROR_SYMBL = 0x002,
    ERROR_BUILD = 0x003,
    ERROR_LR_EXIST = 0X004,
    ERROR_BRACKETS = 0x005,
    ERROR_SCANF    = 0x006,

};

int CountSymbols(const char *filename);

char *ReadToBuffer(const char *filename, int size);

Tree *GetTree(char *buffer, int buffer_size);

void GraphDump(Node *root);


#endif// DIFFERENTIATOR_H