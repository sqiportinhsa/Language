#ifndef TOKENIZATOR
#define TOKENIZATOR

#include "../../Libs/Tree/tree.h"

struct Tokens {
    Tree_node* array = nullptr;
    int         size = 0;
    int     capacity = 0;
};

#endif