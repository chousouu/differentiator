#include "differentiator.h"

int main()
{
    int buffer_size = CountSymbols("example.txt");

    char *buffer = ReadToBuffer("example.txt", buffer_size);

    Tree * DiffTree = GetTree(buffer, buffer_size);
    
    if(DiffTree != NULL)
    {
        printf("root = %lf; %d\n", DiffTree->root->data, DiffTree->root->type);

        InOrder(DiffTree->root);
        
        GraphDump(DiffTree->root);

        TreeDtor(DiffTree);
    }

    free(buffer);
    
    return 0;
}