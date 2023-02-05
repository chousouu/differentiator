#include "differentiator.h"
#include "tree.h"

int CountSymbols(const char *filename)
{
    struct stat buff = {};
    stat(filename, &buff);

    return buff.st_size;
}

char *ReadToBuffer(const char *filename, int size)
{    
    FILE *fp = fopen(filename, "r");

    char *buffer = (char *)calloc(size + 1, sizeof(char));

    if(buffer == NULL)
    {
        printf("buffer = NULL (READTOBUFFER)\n");
    }

    fread(buffer, sizeof(char), size, fp);
    buffer[size] = '\0';
    
    fclose(fp);

    return buffer;
}

static long unsigned HashCounter(void *data, int len)
{
    unsigned long a = 0 , b = 0, c = 0;
    unsigned int fool      = 0xAF00L;
    unsigned int lightning = 1988  ;

    char *datawalker = (char*)data;
    int hash_counter = 0;
    while (len > 4)
    {
        a += fool++;
        b += fool++;
        c += fool++;

        c = a << 2;
        b = a << 3;
        a = a << 1;
        len -= 4;
    }

    c += datawalker[2] << 24;
    b += datawalker[1] << 16;
    a += datawalker[0] << 8 ; 

    hash_counter = a * (fool) + b * (fool + lightning) + c * (fool + 311);

    hash_counter += (hash_counter << 3) | (hash_counter >> 2);
    return hash_counter;
}

static int ProcessAlnum(char *target, Node **node, int *iterator)
{
    if(isalpha(*target))
    {
        char word[4] = {};
        int word_length = 0;
        int read = sscanf(target, "%[^()]%n", word, &word_length);

        DEB("read(%d) : \"%s\" [%d]\n", read, word, word_length);

        if(read == 0) 
        {
            printf("ERROR in reading\n");
            return ERROR_SCANF;
        }

        if(word_length == 1)
        {
            (*node)->type = TYPE_VAR;
            (*node)->data = *target;        
        }
        else // length > 1 ===> cos/ln/....
        {
            (*node)->type = TYPE_UNARY;
            (*node)->data = HashCounter(target, word_length);
            (*iterator) += word_length - 1;
        }
    }
    else  // num
    {
        double num = 0;
        int num_length = 0;
        int read = sscanf(target, "%lf%n", &num, &num_length);

        DEB("read(%d) : %lf [%d]\n", read, num, num_length);

        if(read == 0) 
        {
            printf("ERROR in reading\n");
            return ERROR_SCANF;
        }

        (*node)->type = TYPE_CONST;
        (*node)->data = num;

        (*iterator) += num_length - 1;
    }
    
    return OK;
}

static int ProcessSymbol(char *target, Node **node)
{
    //check value of brackets if smth
    static int brackets = 0;

    switch(*target)
    {
        case '(':
        {
            brackets++;
            DEB("From (type = %d; %lf) : ", (*node)->type, (*node)->data);
            if((*node)->left == NULL)
            {
                DEB("add left, go left\n");
                AddNode(*node, 0, LEFT_NODE);
                *node = (*node)->left;
            }
            else if((*node)->right == NULL && (*node)->type != TYPE_UNARY)
            {
                DEB("add right, go right\n");
                AddNode(*node, 0, RIGHT_NODE);
                *node = (*node)->right;
            }
            else
            {
                printf("ERROR L_R exists");
                return ERROR_LR_EXIST;
            }
            break;
        }
        case ')':
        {
            brackets--;
            DEB("back to parent ([type = %d]%lf)\n", (*node)->parent->type, (*node)->parent->data);
            *node = (*node)->parent;
            break;
        }
        case '+':
        case '-':
        case '*':
        case '^':
        case '/':
        {
            DEB("node operation = %d\n", *target);
            DEB("BEFORE (type = %d) %lf\n", (*node)->type, (*node)->data);  
            (*node)->type = TYPE_OP;
            (*node)->data = *target;
            DEB("after (type = %d) %lf\n", (*node)->type, (*node)->data);  
            break;
        }   
        default: break;
    }

    if(brackets < 0)
    {
        return ERROR_BRACKETS;
    }

    return OK;
}

static int ProcessChar(char *target, Node **node, int *iterator)
{
    DEB("=====\ntarget = \"%c\" ", *target);
    DEB("====node = %p, node->parent = %p\n", *node,(*node)->parent );

    if(isalnum(*target)) // alphabet or num
    {
        DEB("in processalnum\n");
        int alnum_err = ProcessAlnum(target, node, iterator);
        if(alnum_err) return ERROR_ALNUM;
    }
    else
    {
        DEB("in procSymbol\n");
        int symbol_err = ProcessSymbol(target, node);
        if(symbol_err) return ERROR_SYMBL;
    }

    DEB("=======\n");

    return OK;
}

static int DiffTree(char *buffer, Node *node, int buffer_size)
{
    int i = 0;
    for(i = 0; i < buffer_size; i++)
    {
        int error = ProcessChar(buffer + i, &node, &i);
        if(error) break;
    }
    if(i != buffer_size) 
    {
        return ERROR_BUILD;
    }

    return 0;
}

Tree *GetTree(char *buffer, int buffer_size)
{
    Tree *DTree = CreateTree(0);

    if(DiffTree(buffer, DTree->root, buffer_size))
    {
        printf("ERROR in building tree\n");
        TreeDtor(DTree);

        return NULL;
    }

    return DTree;
}

static void DumpNodes(FILE *graph, Node *node, int mode)
{
    if(mode == CREATE_NODE)
    {
        if(node == NULL)
        {
            return;
        }

        fprintf(graph, " \"node%d_%.2lf\" [\n", node->type, node->data);  
            if(node->type == TYPE_CONST)
            {
                fprintf(graph,"label = \"%.2lf\" \n", node->data);
            }
            else 
            {
                fprintf(graph,"label = \"%c\" \n", (int)node->data);
            }
        fprintf(graph, "shape = \"record\"\n"
                       "];\n");

        DumpNodes(graph, node->left, mode);
        DumpNodes(graph, node->right, mode);
    }
    else //if mode == LINK_NODE
    {
        if(node == NULL)
        {
            return;
        }
        if(node->left != NULL)
        {
            fprintf(graph, " \"node%d_%lf\" -> \"node%d_%lf\" [color = \"green\"]\n", 
            node->type, node->data, node->left->type, node->left->data);
        }
        if(node->right != NULL)
        {
            fprintf(graph, " \"node%d_%lf\" -> \"node%d_%lf\" [color = \"red\"]\n", 
            node->type, node->data, node->right->type, node->right->data);
        }
        DumpNodes(graph, node->left, mode);
        DumpNodes(graph, node->right, mode);
    }
}

//TODO: add system calls for opening graph dump
void GraphDump(Node *root)
{
    fclose(fopen("graph.txt", "w"));
    FILE *graph = fopen("graph.txt", "a");

    if(graph == NULL)
    {
        printf("cannot open graph\n");
        return;
    }
    fprintf(graph, "digraph g {\n");

    fprintf(graph, 
                "fontname= \"Helvetica,Arial,sans-serif\"\n"
                "node [fontname=\"Helvetica,Arial,sans-serif\"]\n"
                "edge [fontname=\"Helvetica,Arial,sans-serif\"]\n"
                "graph [rankdir = \"TB\"];\n"
                "node [fontsize = \"16\" shape = \"ellipse\"];\n"
                "edge [];\n"
            );

    DumpNodes(graph, root, CREATE_NODE);
    DumpNodes(graph, root, LINK_NODE);

    fprintf(graph, "}");

   fclose(graph);

    system("dot graph.txt -T png -o dump.png");
}