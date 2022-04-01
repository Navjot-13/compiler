#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "ast.h"

AST* make_node(int type, AST* left, AST* right){
        AST* ast = (AST *)malloc(sizeof(AST));
        ast->type = type;
        ast->left = left;
        ast->right = right;
        ast->datatype = -1;
        ast->symbol = NULL;
        return ast;
}