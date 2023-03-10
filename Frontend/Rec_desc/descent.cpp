#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//#include "descent.h"

#include "../Libs/Tree/tree.h"

const int max_name_len = 3;


//------------------------------------------------------------------------------------------------//
//                                                                                                //
//                                   DECLARATIONS SECTION                                         //
//                                                                                                //
//------------------------------------------------------------------------------------------------//

// GETTING ARGUMENT

static Tree_node* get_argument           (const char **pointer);

static Tree_node* get_value              (const char **pointer);

static Tree_node* get_variable           (const char **pointer);

static Tree_node* get_primary_expression (const char **pointer);


// GETTING FUNCTION
  
static Tree_node* get_add_and_sub   (const char **pointer);
  
static Tree_node* get_mul_and_div   (const char **pointer);

static Tree_node* get_deg           (const char **pointer);


//------------------------------------------------------------------------------------------------//
//                                        GRAMMAR                                                 //
//                                                                                                //
//  Descent                 ::= Get_add_and_sub, '\0';                                            //
//  Get_add_and_sub         ::= Get_mul_and_div {[+, -]}*                                         //
//  Get_mul_and_div         ::= Get_deg {[*, /]}*                                                 //
//  Get_deg                 ::= Get_primary_expression {[^]}*                                     //
//  Get_primary_expression  ::= '(' Get_add_sub ')' | Get_argument                                //
//  Get_argument            ::= Get_value | Get_variable                                          //
//  Get_value               ::= ['0' - '9']+                                                      //
//  Get_variable            ::= ['a' - 'z', 'A' - 'Z']                                            //
//                                                                                                //
//------------------------------------------------------------------------------------------------//

#ifdef DEBUG

#define DEBUG_PRINT(str, ...) {                                             \
    printf ("Func: %20s  line %3d " str, __func__, __LINE__, ##__VA_ARGS__);\
}

#else 

#define DEBUG_PRINT(str, ...) ;

#endif


//------------------------------------------------------------------------------------------------//
//                                                                                                //
//                          MAIN PARSING FUNCTION AND COMMON DEFINES                              //
//                                                                                                //
//------------------------------------------------------------------------------------------------//


#define CHECK(checkup, message, ...)                       \
        if (!(checkup)) {                                  \
            printf("Error: " message, ##__VA_ARGS__);      \
            free_node(node);                               \
            return nullptr;                                \
        }

#define RETURN_NULLPTR_IF(condition) \
        if (condition) {             \
            free_node(node);         \
            return nullptr;          \
        }


Tree_node* descent(const char *pointer) {

    assert(pointer != nullptr);

    DEBUG_PRINT("start\n");

    Tree_node *node = get_add_and_sub(&pointer);

    CHECK(*pointer == '\0', "unexpected symbol: <%c>\n", *pointer);

    return node;
}

//------------------------------------------------------------------------------------------------//
//                                                                                                //
//                              GETTING ARGUMENT (value/varialbe)                                 //
//                                                                                                //
//------------------------------------------------------------------------------------------------//


#define TRY_TO_GET_WITH(got)         \
        node = got;                  \
        if (node != nullptr) {       \
            return node;             \
        }

//-----------------------------//
//        GET ARGUMENT        //
//---------------------------//

static Tree_node* get_argument(const char **pointer) {
    
    assert(pointer != nullptr);

    Tree_node *node = nullptr;
    
    TRY_TO_GET_WITH(get_value   (pointer));
    TRY_TO_GET_WITH(get_variable(pointer));

    CHECK(false, "invalid syntax. Value or variable expected, got <%c>", *pointer);

    return nullptr;
}

//-----------------------------//
//          GET VALUE         //
//---------------------------//

static Tree_node* get_value(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);

    int val = 0;

    const char *pointer_before = *pointer;

    while (isdigit(**pointer)) {

        val = (val * 10) + **pointer - '0';

        ++(*pointer);
    }

    DEBUG_PRINT("got %d\n", val);

    if (pointer_before == *pointer) {
        return nullptr;
    }

    return create_node(val);

}

//-----------------------------//
//       GET VARIABLE         //
//---------------------------//

static Tree_node* get_variable(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);

    if (isalpha(*(*pointer + 0)) && !isalpha(*(*pointer + 1))) {

        Tree_node *node = create_node(**pointer);

        ++(*pointer);

        DEBUG_PRINT("got %c\n", node->data.var);
        
        return node;
    }
    
    return nullptr;
}

//-----------------------------//
//   GET PRIMARY EXPRESSION   //
//---------------------------//

static Tree_node* get_primary_expression(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);

    Tree_node *node = nullptr;

    if (**pointer == '(') {

        ++(*pointer);

        node = get_add_and_sub(pointer);

        RETURN_NULLPTR_IF(node == nullptr);
        CHECK(**pointer == ')', "close bracket ')' expected\n");

        ++(*pointer);

    } else {

        node = get_argument(pointer);
    }

    return node;
}

#undef TRY_TO_GET_WITH

//------------------------------------------------------------------------------------------------//
//                                                                                                //
//                           GETTING FUNCTIONS (transcedent, arithmetic)                          //
//                                                                                                //
//------------------------------------------------------------------------------------------------//

//-----------------------------//
//      GET ADD AND SUB       //
//---------------------------//

static Tree_node* get_add_and_sub(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);

    DEBUG_PRINT("start get add and sub\n");

    Tree_node *node = get_mul_and_div(pointer);

    RETURN_NULLPTR_IF(node == nullptr);

    DEBUG_PRINT("got %d\n", node->data.val);

    while (**pointer == '+' || **pointer == '-') {

        DEBUG_PRINT("cont get add and sub\n");

        Operations op = ADD;

        if (**pointer == '+') { //get type of operation
            op = ADD;
        } else {
            op = SUB;
        }

        ++(*pointer);

        node = create_node(op, node, get_mul_and_div(pointer));

        RETURN_NULLPTR_IF(node        == nullptr);
        RETURN_NULLPTR_IF(node->right == nullptr);
    }

    return node;
}

//-----------------------------//
//      GET MUL AND DIV       //
//---------------------------//

static Tree_node* get_mul_and_div(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);
    
    DEBUG_PRINT("start get mul and div\n");

    Tree_node *node = get_deg(pointer);

    RETURN_NULLPTR_IF(node == nullptr);

    DEBUG_PRINT("got %d\n", node->data.val);

    while (**pointer == '*' || **pointer == '/') {

        DEBUG_PRINT("cont get mul and div\n");

        Operations op = MUL;

        if (**pointer == '*') { //get type of operation
            op = MUL;
        } else {
            op = DIV;
        }

        ++(*pointer);

        node = create_node(op, node, get_deg(pointer));

        RETURN_NULLPTR_IF(node        == nullptr);
        RETURN_NULLPTR_IF(node->right == nullptr);
    }

    return node;
}

//-----------------------------//
//         GET DEGREE         //
//---------------------------//

static Tree_node* get_deg(const char **pointer) {

    assert( pointer != nullptr);
    assert(*pointer != nullptr);

    DEBUG_PRINT("start get deg\n");

    Tree_node *node = get_primary_expression(pointer);

    RETURN_NULLPTR_IF(node == nullptr);

    DEBUG_PRINT("got %d\n", node->data.val);

    while (**pointer == '^') {

        DEBUG_PRINT("cont get deg\n");

        ++(*pointer);

        node = create_node(DEG, node, get_primary_expression(pointer));

        RETURN_NULLPTR_IF(node        == nullptr);
        RETURN_NULLPTR_IF(node->right == nullptr);
    }

    return node;
}

#undef CHECK_IF_FUNC_IS

#undef WARNING
#undef RETURN_FALSE_IF
