#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "ast.h"

AST* make_node(int type, AST *child1, AST *child2, AST *child3, AST *child4) {
        AST* ast = (AST *)malloc(sizeof(AST));
        ast->type = type;
        ast->child[0] = child1;
        ast->child[1] = child2;
        ast->child[2] = child3;
        ast->child[3] = child4;
        ast->datatype = -1;
        ast->symbol = NULL;
        ast->size = -1;
        ast->reg = 0;
        ast->freg = 0;
        return ast;
}