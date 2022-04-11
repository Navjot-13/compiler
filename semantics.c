#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/ast.h"
#include "Utils/symbol_table.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;
extern SymbolTable* global_symbol_table;
extern SymbolTable *symbol_table;
FILE *fp;

void assign_type (AST *astroot);
void traverse(AST *astroot);
void typecheck(AST *astroot);
void binary_op_type_checking(AST *astroot);
void add_params(AST* astroot);
void check_params(AST* astroot);
bool compatible_types(int type1,int type2);

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
    global_symbol_table = symbol_table;
    // stack = NULL;
    yyparse();
    fp = fopen("assembly.asm","w+");
    fprintf(fp,"    .data\n");
    fprintf(fp,"    .text\n");
    fprintf(fp,"    .globl main\n");
    fprintf(fp,"begin:\n");
    traverse(astroot);
}

void traverse(AST *astroot)
{
    if (astroot == NULL)
        return;

    switch (astroot->type)
    {
        case ast_var_list:
        {
            for (int i = 0; i < 4; i++) {
                if (astroot->child[i] != NULL) {
                    astroot->child[i]->datatype = astroot->datatype;
                    if (astroot->child[i]->type == ast_variable_stmt) {
                        astroot->child[i]->symbol->type = astroot->datatype;
                    }
                }
            }
            break;
        }
    }

    for (int i = 0; i < 4; i++)
        traverse(astroot->child[i]);

    switch (astroot->type)
    {
        case ast_stmts:
        {
            break;
        }
        case ast_push_scope:
        {
            push_symbol_table();
            break;
        }
        case ast_pop_scope:
        {
            pop_symbol_table();
            break;
        }
        case ast_start_stmt:
        {
            break;
        }
        case ast_func_stmt:
        {
            add_params(astroot);
            break;
        }
        case ast_func_list_stmt:
        {
            break;
        }
        case ast_param_list_stmt:
        {
            break;
        }
        case ast_param_stmt:
        {
            push_symbol(astroot->symbol);
            break;
        }
        case ast_arg_list_stmt:
        {
            break;
        }
        case ast_func_call_stmt:
        {
            check_params(astroot);
            break;
        }
        case ast_stmt_list:
        {
            break;
        }
        case ast_assgn_stmt:
        {
            astroot->val = astroot->child[1]->val;
            astroot->datatype = astroot->child[0]->datatype;
            typecheck(astroot);
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
            break;
        }
        case ast_array_decl_stmt:
        {
            push_symbol(astroot->symbol);
            printf("Array size: %d\n",astroot->symbol->size);
            break;
        }
        case ast_expressions_stmt:
        {

            break;
        }
        case ast_arry_assgn_stmt:
        {
            Symbol *symbol = search_symbol(astroot->symbol->name);
            if(symbol == NULL){
                printf("Identifier undeclared\n");
            }
            if(symbol->is_array != 1){
                printf("Identifier not an array type.\n");
                exit(0);
            }
            if(astroot->child[1]->datatype != INT_TYPE){
                printf("array subscript not an integer\n");
                exit(0);
            }
            astroot->datatype = symbol->type;
            break;
        }
        case ast_array_stmt:
        {
            break;
        }
        case ast_variable_stmt:
        {
            astroot->symbol->type = astroot->datatype;
            Symbol* symbol = search_symbol(astroot->symbol->name);
            if(symbol != NULL){
                printf("Identified %s already declared in current scope\n",symbol->name);
                exit(0);
            }
            push_symbol(astroot->symbol);
            fprintf(fp,"    la $sp, -8($sp)\n");
            fprintf(fp,"    sw $fp, 4($sp)\n");
            fprintf(fp,"    sw $ra, 0($sp)\n");
            fprintf(fp,"    la $fp, 0($sp)\n");
            fprintf(fp,"    la $sp, -4($sp)\n");
            break;
        }
        case ast_var_list:
        {
            break;
        }
        case ast_var_expr:
        {
            Symbol *symbol = search_symbol(astroot->symbol->name);
            if (symbol == NULL)
            {
                printf("\nError: Variable %s not declared\n", astroot->symbol->name);
                exit(0);
            }
            astroot->symbol = symbol;
            astroot->datatype = symbol->type;
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
            binary_op_type_checking(astroot);
            fprintf(fp,"    la $s1, -4($fp)\n");
            fprintf(fp,"    lw $s2, ($s1)\n");
            break;
        }
        case ast_sub_stmt:
        {
            binary_op_type_checking(astroot);
            break;
        }
        case ast_mul_stmt:
        {
            binary_op_type_checking(astroot);
            break;
        }
        case ast_div_stmt:
        {
            binary_op_type_checking(astroot);
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
        case ast_const_val:
        {
            
            break;
        }
        case ast_print_stmt:
        {

        }
        case ast_input_stmt:
        {
            Symbol *symbol = search_symbol(astroot->symbol->name);
            if(symbol == NULL){
                printf("Identifier undeclared\n");
                exit(0);
            }
        }
        default:
        {
            break;
        }
    }
}

void typecheck(AST *astroot) {
    if (astroot->child[0]->datatype == INT_TYPE && astroot->child[1]->datatype == DOUBLE_TYPE) {
        astroot->child[1]->val.int_val= (int)astroot->child[1]->val.double_val;
        astroot->datatype = INT_TYPE;
    } else if (astroot->child[0]->datatype == DOUBLE_TYPE && astroot->child[1]->datatype == INT_TYPE) {
        astroot->child[0]->val.double_val= (double)astroot->child[1]->val.int_val;
        astroot->datatype = DOUBLE_TYPE;
    } else if(astroot->child[0]->datatype != astroot->child[1]->datatype) {
        printf("\nError: Type mismatch in assignment statement\n");
        exit(0);
    }
}

void binary_op_type_checking(AST *astroot){
    if(astroot->child[0]->datatype == DOUBLE_TYPE && astroot->child[1]->datatype == DOUBLE_TYPE){
        astroot->datatype = DOUBLE_TYPE;
        astroot->val.double_val = astroot->child[0]->val.double_val + astroot->child[1]->val.double_val;
    } else if(astroot->child[0]->datatype == INT_TYPE && astroot->child[1]->datatype == DOUBLE_TYPE){
        astroot->datatype = DOUBLE_TYPE;
        astroot->val.double_val = astroot->child[0]->val.int_val + astroot->child[1]->val.double_val;
    } else if(astroot->child[0]->datatype == DOUBLE_TYPE && astroot->child[1]->datatype == INT_TYPE){
        astroot->datatype = DOUBLE_TYPE;
        astroot->val.double_val = astroot->child[0]->val.double_val + astroot->child[1]->val.int_val;
    } else if(astroot->child[0]->datatype == INT_TYPE && astroot->child[1]->datatype == INT_TYPE) {
        astroot->datatype = DOUBLE_TYPE;
        astroot->val.int_val = astroot->child[0]->val.int_val + astroot->child[1]->val.int_val;
    } else{
        printf("Invalid operands for +");
        exit(0);
    }
}

void add_params(AST* astroot){
    AST* params = astroot->child[1];
    Symbol* func = astroot->symbol;
    if(global_search_symbol(func->name) != NULL){
        printf("Function with name %s already exists\n",func->name);
        exit(0);
    }
    Symbol* cur_param = func->param_list;
    while(params != NULL){
        char* name = params->child[1]->symbol->name;
        int type = params->child[1]->symbol->type;
        if(cur_param == NULL){
            func->param_list = symbol_init(name,type,NULL,NULL);
            cur_param = func->param_list;
        } else{
            cur_param->next = symbol_init(name,type,NULL,NULL);
            cur_param->next->prev = cur_param;
            cur_param = cur_param->next;
        }
        params = params->child[0];
    }
    push_global_symbol(func);// push the function symbol to the global scope
}

// check if implicit type conversion can take place between 2 given types
bool convertible_types(int type1,int type2){
    if(type1 == type2){
        return true;
    }
    if(type1 == STR_TYPE || type2 == STR_TYPE){
        return false;
    }
    return true;
}

void check_params(AST* astroot){
    AST* args = astroot->child[0];
    Symbol* func = global_search_symbol(astroot->symbol->name);
    if(func == NULL){
        printf("No function with name %s exists.\n",astroot->symbol->name);
        exit(0);
    }
    Symbol* params = func->param_list;
    while(params != NULL && args != NULL){
        if(!convertible_types(params->type,args->child[1]->datatype)){
            printf("Type mismatch in function %s for parameter %s\n",astroot->symbol->name,params->name);
            exit(0);
        }
        params = params->next;
        args = args->child[0];
    }
    if(params != NULL){
        printf("Type: %s\n",params->name);
        printf("Too few arguments for the function %s\n",astroot->symbol->name);
        exit(0);
    } else if(args != NULL){
        printf("Too many arguments for the function %s\n",astroot->symbol->name);
        exit(0);
    }
    printf("Return type : %d\n",func->type);
    astroot->datatype = func->type;
}