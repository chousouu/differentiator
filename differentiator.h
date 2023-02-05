#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "tree.h"

#define DEB(...) printf(__VA_ARGS__)

#define PRINT_UNARY(PRINT_X, X) else if(node->data == X) { fprintf(graph,"label = \" %s\"\n", #PRINT_X);}

enum UNARY
{
    UNARY_COS = 'soc',
    UNARY_SIN = 'nis',
    UNARY_LN  = 'nl',
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
    ERROR_UNARY    = 0x007,

};

int CountSymbols(const char *filename);

char *ReadToBuffer(const char *filename, int size);

Tree *GetTree(char *buffer, int buffer_size);

void GraphDump(Node *root);


#endif// DIFFERENTIATOR_H