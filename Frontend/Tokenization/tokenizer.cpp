#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "../../Libs/File_reading/file_reading.hpp"

const int start_capacity = 16;

#define memory_allocate(ptr, size, type)                                \
        ptr = (type*) calloc(size, sizeof(type));                       \
        if (ptr == nullptr) {                                           \
            printf("can't allocate memory: not enought free mem\n");    \
            return;                                                     \
        }

void init_tokens(Tokens *tokens) {

    assert(tokens != nullptr);

    memory_allocate(tokens->array, start_capacity, Tree_node);

    tokens->capacity = start_capacity;
    tokens->size     = 0;
}

void resize_tokens(Tokens *tokens) {

    assert(tokens != nullptr);

    realloc(tokens->array, tokens->capacity * 2);

    tokens->capacity *= 2;
}

void destruct_tokens(Tokens *tokens) {

    assert(tokens != nullptr);

    free(tokens->array);

    tokens->size     = 0;
    tokens->capacity = 0;
}

#define TRY(func)                                            \
        if (func(&origin, &tokens->array[tokens->size])) {   \
            ++(tokens->size);                                \
            continue;                                        \
        }

void tokenize(char *origin, Tokens *tokens) {
    assert(tokens != nullptr);
    assert(origin != nullptr);

    skip_comments(&origin);
    skip_spaces  (&origin);

    while (*origin != '\0') {
        if (tokens->capacity == tokens->size) {
            resize_tokens(tokens);
        }

        TRY(scan_value);
        TRY(scan_oper);
    }
}

void skip_comments(char **origin) {

    assert( origin != nullptr);
    assert(*origin != nullptr);

    if (**origin == '[') {
        for (; **origin != ']'; ++(*origin));

        ++(*origin);
    }
}

bool scan_value(char **origin, Tree_node *node) {
    assert( origin != nullptr);
    assert(*origin != nullptr);

    int val = 0;
    int len = 0;

    sscanf(*origin, "%d%n", val, len);

    if (len == 0) {
        return false;
    }

    node->type     = VALUE;
    node->data.val = val;
    node->origin   = *origin;

    *origin += len;

    return true;
}


#define TRY_GET_OP(oper, num, len)                \
        if (strncasecmp(oper, sym, len) == 0) {   \
            node->type    = OPERATOR;             \
            node->data.op = num;                  \
            node->origin  = *origin;              \
            (*origin) += len;                     \
            return true;                          \
        }

bool scan_oper(char **origin, Tree_node *node) {
    assert( origin != nullptr);
    assert(*origin != nullptr);

    char sym[2] = {};
    sscanf(*origin, "%c%c", sym[0], sym[1]);

    TRY_GET_OP("+",  ADD,  1);
    TRY_GET_OP("-",  SUB,  1);
    TRY_GET_OP("*",  MUL,  1);
    TRY_GET_OP("/",  DIV,  1);
    TRY_GET_OP("^",  DEG,  1);
    TRY_GET_OP(">",  MORE, 1);
    TRY_GET_OP("<",  LESS, 1);
    TRY_GET_OP("!",  NOT,  1);
    TRY_GET_OP("=",  ASS,  1);    
    TRY_GET_OP("&&", AND,  2);
    TRY_GET_OP("||", OR,   2);
    TRY_GET_OP("==", EQ,   2);
    TRY_GET_OP(">=", MEQ,  2);
    TRY_GET_OP("<=", LEQ,  2);
    TRY_GET_OP("!=", UNEQ, 2);

    return false;    
}
