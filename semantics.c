#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/ast.h"
#include "Utils/symbol_table.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;
extern SymbolTable *symbol_table;

void traverse(AST *astroot);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("\nUsage: <exefile> <inputfile>\n");
        exit(0);
    }
    yyin = fopen(argv[1], "r");
    symbol_table = (SymbolTable *)malloc(sizeof(SymbolTable));
    symbol_table->prev = NULL;
    symbol_table->next = NULL;
    symbol_table->symbol_head = NULL;
    stack = NULL;
    yyparse();
    traverse(astroot);
}

void traverse(AST *astroot)
{
    if (astroot == NULL)
        return;

    switch (astroot->type)
    {
        case ast_stmt_list:
        {
            printf("ast_stmt_list\n");
            break;
        }
        case ast_assgn_stmt:
        {
            printf("ast_assgn_stmt\n");
            break;
        }
        case ast_cond_stmt:
        {
            break;
        }
        case ast_loop_stmt:
        {
            break;
        }
        case ast_decl_stmt:
        {
            printf("ast_decl_stmt\n");
            break;
        }
        case ast_array_decl_stmt:
        {
            break;
        }
        case ast_expressions_stmt:
        {
            break;
        }
        case ast_arry_assgn_stmt:
        {
            break;
        }
        case ast_variable_stmt:
        {
            push_symbol(astroot->symbol);
            break;
        }
        case ast_var_list:
        {
            astroot->left->datatype = astroot->datatype;
            astroot->right->datatype = astroot->datatype;
            astroot->left->symbol->type = astroot->datatype;
            astroot->right->symbol->type = astroot->datatype;`
            break;
        }
        case ast_aeq_stmt:
        {
            break;
        }
        case ast_seq_stmt:
        {
            break;
        }
        case ast_meq_stmt:
        {
            break;
        }
        case ast_deq_stmt:
        {
            break;
        }
        case ast_incr_stmt:
        {
            break;
        }
        case ast_decr_stmt:
        {
            break;
        }
        case ast_or_stmt:
        {
            break;
        }
        case ast_and_stmt:
        {
            break;
        }
        case ast_eq_stmt:
        {
            break;
        }
        case ast_neq_stmt:
        {
            break;
        }
        case ast_lt_stmt:
        {
            break;
        }
        case ast_gt_stmt:
        {
            break;
        }
        case ast_geq_stmt:
        {
            break;
        }
        case ast_leq_stmt:
        {
            break;
        }
        case ast_add_stmt:
        {
            break;
        }
        case ast_sub_stmt:
        {
            break;
        }
        case ast_mul_stmt:
        {
            break;
        }
        case ast_div_stmt:
        {
            break;
        }
        case ast_unary_not:
        {
            break;
        }
        case ast_unary_add:
        {
            break;
        }
        case ast_unary_sub:
        {
            break;
        }
    }
    traverse(astroot->left);
    traverse(astroot->right);
}
