#ifndef AST_H
#define AST_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "symbol_table.h"

#define ast_stmt_list 0
#define ast_assgn_stmt 1
#define ast_cond_stmt 2
#define ast_loop_stmt 3
#define ast_array_decl_stmt 4
#define ast_expressions_stmt 5
#define ast_arry_assgn_stmt 6
#define ast_variable_stmt 7
#define ast_aeq_stmt 8
#define ast_seq_stmt 9
#define ast_meq_stmt 10
#define ast_deq_stmt 11
#define ast_incr_stmt 12
#define ast_decr_stmt 13
#define ast_or_stmt 14
#define ast_and_stmt 15
#define ast_eq_stmt 17
#define ast_neq_stmt 18
#define ast_lt_stmt 19
#define ast_gt_stmt 20
#define ast_geq_stmt 21
#define ast_leq_stmt 22
#define ast_add_stmt 23
#define ast_sub_stmt 24
#define ast_mul_stmt 25
#define ast_div_stmt 26
#define ast_unary_not 27
#define ast_unary_add 28
#define ast_unary_sub 29

typedef struct AST{
        int type;
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

AST* make_node(int type, AST* left, AST* right);

#endif