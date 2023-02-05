#include "differentiator.h"
#include "tree.h"

#define N_UNARY_FUNCS 3
int UNARY_FUNCS[] = {'soc', 'nis', 'nl'};


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

static long unsigned HashUnary(void *data, int len)
{
    char *datawalker = (char*)data;
    int hash_counter = 0;

    int two_five_six_n = 1;
    for(int i = 0; i < len; i++)
    {
        hash_counter += (datawalker[i] * (two_five_six_n));
        two_five_six_n *= 256; 
    }
 
    return hash_counter;
}

static int isUnary(int HASHED_UNARY)
{    
    for(int i = 0; i < N_UNARY_FUNCS; i++)
    {
        if(HASHED_UNARY == UNARY_FUNCS[i])
        {
            return i;
        }
    }

    return -1;
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
            int hashed_unary = HashUnary(target, word_length);
            if(isUnary(hashed_unary) < 0)
            {
                printf("non existing unary func\n");
                return ERROR_UNARY;
            }
            
            (*node)->type = TYPE_UNARY;
            (*node)->data = hashed_unary;
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
    Tree *DTree = CreateTree(66);

    if(DiffTree(buffer, DTree->root, buffer_size))
    {
        printf("ERROR in building tree\n");
        TreeDtor(DTree);

        return NULL;
    }

    if(DTree->root->right == NULL)  
    { // with no right tree exists additional node (root)
        Node *tmp = DTree->root;
        DTree->root = DTree->root->left;

        free(tmp);
    }

    return DTree;
}

static void DumpCreateNodes(FILE *graph, Node *node)
{
    if(node == NULL)
    {
        return;
    }
    if(!(node->data == 0 && node->type == 0))
    {
    
    fprintf(graph, " \"node%p\" [\n", node);  
        if(node->type == TYPE_CONST)
        {
            fprintf(graph,"label = \"%.2lf\" \n", node->data);
        }
        else if (node->type == TYPE_UNARY)
        {
            if(node->data == 'nis')
            {
                fprintf(graph,"label = \"sin\" \n");
            }
            PRINT_UNARY(cos, 'soc')
            PRINT_UNARY(ln, 'nl')
        } 
        else 
        {
            fprintf(graph,"label = \"%c\" \n", (int)node->data);
        }
    fprintf(graph, "shape = \"record\"\n"
                    "];\n");
    }
    DumpCreateNodes(graph, node->left);
    DumpCreateNodes(graph, node->right);

    return;
}


static void DumpLinkNodes(FILE *graph, Node *node)
{
    if(node == NULL)
    {
        return;
    }
    if (node->left != NULL)
    {
        fprintf(graph, " \"node%p\" -> \"node%p\" [color = \"green\"]\n", 
        node, node->left);
    }
    if(node->right != NULL)
    {
        fprintf(graph, " \"node%p\" -> \"node%p\" [color = \"red\"]\n", 
        node, node->right);
    }
    

    DumpLinkNodes(graph, node->left);
    DumpLinkNodes(graph, node->right);
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

    DumpCreateNodes(graph, root);
    DumpLinkNodes(graph, root);

    fprintf(graph, "}");

    fclose(graph);

    system("dot graph.txt -T png -o dump.png");
}