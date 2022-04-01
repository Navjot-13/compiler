#ifndef AST_H
#define AST_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct AST{
        char *op;
        bool is_unary;
        bool is_leaf;
        bool is_array;
        bool is_expressions;
        bool is_stmt_list;
        bool is_assign_stmt;
        bool is_cond_stmt;
        bool is_loop_stmt;
        bool is_array_stmt;
        bool is_condition_stmt;
        bool is_expression_stmt;
        int datatype;
        Symbol *symbol;
        union {
                int int_val;
                double double_val;
                char str_val[300];
        } val;
        struct AST *left;
        struct AST *right;

} AST;

AST* make_node(char *op, AST* left, AST* right){
        AST* ast = (AST *)malloc(sizeof(AST));
        ast->op = (char*)malloc((strlen(op)+1)*sizeof(char));
        strcpy(ast->op,op);
        ast->left = left;
        ast->right = right;
        ast->is_unary = false;
        ast->is_leaf = false;
        ast->is_expressions = false;
        ast->is_stmt_list = false;
        ast->is_assign_stmt = false;
        ast->is_cond_stmt = false;
        ast->is_loop_stmt = false;
        ast->is_array_stmt = false;
        ast->is_condition_stmt = false;
        ast->datatype = -1;
        ast->symbol = NULL;
        return ast;
}

#endif