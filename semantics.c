#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/ast.h"
#include "Utils/symbol_table.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;
extern SymbolTable* current_symbol_table;
extern SymbolTable* persistent_symbol_table;
extern int current_scope;
extern int unused_scope;
FILE *fp;

int registers[18];      // Registers from $8 to $25
int lru_counter[18];
int global_counter = 1;
int fregisters[18];     // float registers
int lru_fcounter[18];
int global_fcounter = 1;
int global_offset = 0;
int label = 0;
void assign_type (AST *astroot);
void traverse(AST *astroot);
void typecheck(AST *astroot);
void binary_op_type_checking(AST *astroot);
void add_params(AST* astroot);
void check_params(AST* astroot);
bool compatible_types(int type1,int type2);
void generate_code(AST* astroot);
int get_size(int type);
void update_counter();
void update_register(int index);
int get_register();
int get_fregister();

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("\nUsage: <exefile> <inputfile>\n");
        exit(0);
    }
    yyin = fopen(argv[1], "r");
    yyparse();
    current_symbol_table = NULL;
    persistent_symbol_table = NULL;
    current_scope = -1;
    unused_scope = 0;
    current_scope = unused_scope;
    ++unused_scope;
    
    fp = fopen("assembly.asm","w+");
    fprintf(fp,"    .data\n");
    fprintf(fp,"    .text\n");
    fprintf(fp,"    .globl main\n");
    fprintf(fp,"main:\n");
    fprintf(fp,"    la $fp, 0($sp)\n");
    astroot->next = label++;
    traverse(astroot);
    fprintf(fp,"    jr $ra\n");
    fclose(fp);

    for (int i = 0; i < 18; ++i)
    {
        printf("%d ", lru_counter[i]);
    }
    printf("\n");
}

void traverse_ast_stmts(AST* astroot)
{
    if(astroot->child[1]){
        astroot->child[1]->next = astroot->next;
    }
    for(int i = 0; i < 4;++i){
        traverse(astroot->child[i]);
    }
    // fprintf(fp,"__%d__:\n",astroot->next);
}

void traverse_ast_push_scope(AST* astroot)
{
    push_symbol_table();
}

void traverse_ast_pop_scope(AST* astroot)
{
    pop_symbol_table();
}

void traverse_ast_start_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_func_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    add_params(astroot);
}

void traverse_func_list_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_param_list_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_param_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    astroot->symbol->size = get_size(astroot->symbol->type);
    push_symbol(astroot->symbol);
}

void traverse_ast_arg_list_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_func_call_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    check_params(astroot);
}

void traverse_ast_stmt_list(AST* astroot)
{
    astroot->child[0]->next = label++;
    if(astroot->child[1]){
        astroot->child[1]->next = astroot->next;
    }
    traverse(astroot->child[0]);
    if(astroot->child[0]->type == ast_cond_stmt || astroot->child[0]->type == ast_loop_stmt){
        fprintf(fp,"__%d__:\n",astroot->child[0]->next);
    }
    traverse(astroot->child[1]);
}

void traverse_ast_assgn_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    // printf("Type 1: %d Type 2: %d\n",astroot->child[0]->type,astroot->child[1]->type);
    // printf("Datatype of left child: %d\n",astroot->child[0]->datatype);
    astroot->val = astroot->child[1]->val;
    astroot->datatype = astroot->child[0]->datatype;  
    typecheck(astroot);
    // Generate Code (Considering only integers for now)
    printf("%s is the register with offset %d\n", astroot->child[0]->symbol->name, astroot->child[0]->symbol->offset);
    fprintf(fp, "    sw $%d, -%d($fp)\n", astroot->child[1]->reg, astroot->child[0]->symbol->offset);
    astroot->reg = astroot->child[1]->reg;
}
        
void traverse_ast_cond_stmt(AST* astroot)
{
    
    // check that it is not an else construct
    if(astroot->child[0]->child[0]){
        astroot->child[0]->child[0]->tru = label++;
        // check if it has a corresponding else if/else 
        if(astroot->child[1]){
            astroot->child[0]->child[0]->fal = label++;
            astroot->child[0]->child[2]->next = astroot->child[1]->next = astroot->next;
            traverse(astroot->child[0]->child[0]);
            traverse(astroot->child[0]->child[1]);
            fprintf(fp,"   beqz $%d __%d__\n",astroot->child[0]->child[0]->reg,astroot->child[0]->child[0]->fal);
            // fprintf(fp,"   j __%d__\n",astroot->child[0]->child[0]->fal);
            // fprintf(fp,"__%d__:\n",astroot->child[0]->child[0]->tru);
            traverse(astroot->child[0]->child[2]);
            fprintf(fp,"    j __%d__\n",astroot->next);
            fprintf(fp,"__%d__:\n",astroot->child[0]->child[0]->fal);
            traverse(astroot->child[0]->child[3]);
            traverse(astroot->child[1]);
        }
        else{
            astroot->child[0]->child[0]->fal = astroot->child[0]->child[2]->next = astroot->next;
            traverse(astroot->child[0]->child[0]);
            traverse(astroot->child[0]->child[1]);
            fprintf(fp,"   beqz $%d __%d__\n",astroot->child[0]->child[0]->reg,astroot->child[0]->child[0]->fal);
            // fprintf(fp,"   j __%d__\n",astroot->child[0]->child[0]->fal);
            // fprintf(fp,"__%d__:\n",astroot->child[0]->child[0]->tru);
            traverse(astroot->child[0]->child[2]);
            traverse(astroot->child[0]->child[3]);
        }
    }
    else{
        for(int i = 0; i < 4;++i){
            traverse(astroot->child[i]);
        }
    }
}
      
void traverse_ast_loop_stmt(AST* astroot)
{
    int begin = label++;
    astroot->child[0]->tru = label++;
    astroot->child[0]->fal = astroot->next;
    astroot->child[1]->next = begin;
    fprintf(fp,"__%d__:\n",begin);
    traverse(astroot->child[0]);
    fprintf(fp,"    beqz $%d, __%d__\n",astroot->child[0]->reg,astroot->next);
    // fprintf(fp,"__%d__:\n",astroot->child[0]->tru);
    traverse(astroot->child[1]);
    fprintf(fp,"    j __%d__\n",begin);
}
        
void traverse_ast_decl_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}
        
void traverse_ast_array_decl_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    astroot->symbol->size = get_size(astroot->symbol->type) * astroot->symbol->size;
    push_symbol(astroot->symbol);
    printf("Array size: %d\n",astroot->symbol->size);
}

void traverse_ast_expressions_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}
        
void traverse_ast_arry_assgn_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
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
}

void traverse_ast_array_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_variable_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->symbol->type = astroot->datatype;
    Symbol* symbol = search_symbol(astroot->symbol->name);
    if(symbol != NULL){
        printf("Identified %s already declared in current scope\n",symbol->name);
        exit(0);
    }
    astroot->symbol->size = get_size(astroot->symbol->type);
    astroot->symbol->offset = global_offset;
    global_offset += astroot->symbol->size;
    push_symbol(astroot->symbol);
    // Generate Code
    fprintf(fp,"    la $sp, -%d($sp)\n",get_size(astroot->symbol->type));// allocate stack frame
}

void traverse_ast_var_list(AST* astroot)
{
    for (int i = 0; i < 4; i++) {
            if (astroot->child[i] != NULL) {
            astroot->child[i]->datatype = astroot->datatype;
            if (astroot->child[i]->type == ast_variable_stmt) {
                astroot->child[i]->symbol->type = astroot->datatype;
            }
        }
    }
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_var_expr(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    Symbol *symbol = search_symbol(astroot->symbol->name);
    if (symbol == NULL)
    {
        printf("\nError: Variable %s not declared\n", astroot->symbol->name);
        exit(0);
    }
    astroot->symbol = symbol;
    astroot->datatype = symbol->type;

    // Generate code
    int reg = get_register();
    astroot->reg = reg;
    fprintf(fp, "    lw $%d, -%d($fp)\n", reg, astroot->symbol->offset);
}

void traverse_ast_or_stmt(AST* astroot)
{
    astroot->child[0]->tru = astroot->tru;
    astroot->child[0]->fal = label++;
    astroot->child[1]->tru = astroot->tru;
    astroot->child[1]->fal = astroot->fal;
    for(int i = 0; i < 4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sne $%d, $%d, $0\n", reg0, reg0);
    fprintf(fp, "    sne $%d, $%d, $0\n", reg1, reg1);
    fprintf(fp, "    or $%d, $%d, $%d\n", reg0, reg0, reg1);

}

void traverse_ast_and_stmt(AST* astroot)
{
    astroot->child[0]->tru = label++;
    astroot->child[0]->fal = astroot->fal;
    astroot->child[1]->tru = astroot->tru;
    astroot->child[1]->fal = astroot->fal;
    for(int i = 0; i < 4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sne $%d, $%d, $0\n", reg0, reg0);
    fprintf(fp, "    sne $%d, $%d, $0\n", reg1, reg1);
    fprintf(fp, "    and $%d, $%d, $%d\n", reg0, reg0, reg1);
}
    
void traverse_ast_eq_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    seq $%d, $%d, $%d\n", reg0, reg0, reg1);
}
        
void traverse_ast_neq_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sne $%d, $%d, $%d\n", reg0, reg0, reg1);
}
        
void traverse_ast_lt_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    slt $%d, $%d, $%d\n", reg0, reg0, reg1);
}
        
void traverse_ast_gt_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sgt $%d, $%d, $%d\n", reg0, reg0, reg1);

}

void traverse_ast_geq_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sge $%d, $%d, $%d\n", reg0, reg1, reg0);
}

void traverse_ast_leq_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    update_register(reg0);
    update_register(reg1);
    astroot->reg = reg0;
    fprintf(fp, "    sle $%d, $%d, $%d\n", reg0, reg1, reg0);
}

void traverse_ast_add_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    binary_op_type_checking(astroot);
    
    // Generate Code (Considering only integers for now)
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    astroot->reg = reg0;
    fprintf(fp, "    add $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
    update_register(reg0);
    update_register(reg1);
}

void traverse_ast_sub_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    astroot->reg = reg0;
    fprintf(fp, "    sub $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
    update_register(reg0);
    update_register(reg1);
}

void traverse_ast_mul_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->scope_no = current_scope;
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    astroot->reg = reg0;
    fprintf(fp, "    mul $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
    update_register(reg0);
    update_register(reg1);
}

void traverse_ast_div_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    int reg0 = astroot->child[0]->reg;
    int reg1 = astroot->child[1]->reg;
    astroot->reg = reg0;
    fprintf(fp, "    div $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
    update_register(reg0);
    update_register(reg1);
}

void traverse_ast_unary_not(AST* astroot)
{
    astroot->child[0]->tru = astroot->fal;
    astroot->child[0]->fal = astroot->tru;
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_unary_add(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;
}

void traverse_ast_unary_sub(AST* root)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;
}

void traverse_ast_const_val(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg1 = get_register();
        astroot->reg = reg1;
        fprintf(fp, "    li $%d, %d\n", reg1, astroot->val.int_val);
    }
    if(astroot->datatype == DOUBLE_TYPE){
        int freg1 = get_fregister();
        printf("in data type");
        astroot->freg = freg1;
        fprintf(fp, "   l.s $f%d, %lf\n", freg1, astroot->val.double_val);
    }
    if(astroot->datatype == BOOL_TYPE){

    }
    if(astroot->datatype == STR_TYPE){
          
    }
    
}

void traverse_ast_print_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_input_stmt(AST* astroot)
{
    astroot->scope_no = current_scope;
    Symbol *symbol = search_symbol(astroot->child[0]->symbol->name);
    if(symbol == NULL){
        printf("Identifier undeclared\n");
        exit(0);
    }

    // For integer
    if (symbol->type == INT_TYPE) {
        fprintf(fp, "    li $v0, 5\n");
        fprintf(fp, "    syscall\n");

        int reg1 = get_register();
        astroot->reg = reg1;
        fprintf(fp, "    move $%d, $v0\n", reg1);
        update_register(reg1);
    }
}

void traverse_ast_return_stmt(AST* astroot)
{
    // pass
}



void traverse(AST *astroot)
{
    if (astroot == NULL)
        return;

    switch (astroot->type)
    {
        case ast_stmts:
        {
            traverse_ast_stmts(astroot);
            break;
        }
        case ast_push_scope:
        {
            traverse_ast_push_scope(astroot);
            break;
        }
        case ast_pop_scope:
        {
            traverse_ast_pop_scope(astroot);
            break;
        }
        case ast_start_stmt:
        {
            traverse_ast_start_stmt(astroot);
            break;
        }
        case ast_func_stmt:
        {
            traverse_ast_func_stmt(astroot);
            break;
        }
        case ast_func_list_stmt:
        {
            traverse_func_list_stmt(astroot);
            break;
        }
        case ast_param_list_stmt:
        {
            traverse_ast_param_list_stmt(astroot);
            break;
        }
        case ast_param_stmt:
        {
            traverse_ast_param_stmt(astroot);
            break;
        }
        case ast_arg_list_stmt:
        {
            traverse_ast_arg_list_stmt(astroot);
            break;
        }
        case ast_func_call_stmt:
        {
            traverse_ast_func_call_stmt(astroot);
            break;
        }
        case ast_stmt_list:
        {
            traverse_ast_stmt_list(astroot);
            break;
        }
        case ast_assgn_stmt:
        {
            traverse_ast_assgn_stmt(astroot);
            break;
        }
        case ast_cond_stmt:
        {
            traverse_ast_cond_stmt(astroot);
            break;
        }
        case ast_loop_stmt:
        {
            traverse_ast_loop_stmt(astroot);
            break;
        }
        case ast_decl_stmt:
        {
            traverse_ast_decl_stmt(astroot);
            break;
        }
        case ast_array_decl_stmt:
        {
            traverse_ast_array_decl_stmt(astroot);
            break;
        }
        case ast_expressions_stmt:
        {
            traverse_ast_expressions_stmt(astroot);
            break;
        }
        case ast_arry_assgn_stmt:
        {
            traverse_ast_arry_assgn_stmt(astroot);
            break;
        }
        case ast_array_stmt:
        {
            traverse_ast_array_stmt(astroot);
            break;
        }
        case ast_variable_stmt:
        {
            traverse_ast_variable_stmt(astroot);
            break;
        }
        case ast_var_list:
        {
            traverse_ast_var_list(astroot);
            break;
        }
        case ast_var_expr:
        {
            traverse_ast_var_expr(astroot);
            break;
        }
        case ast_or_stmt:
        {
            traverse_ast_or_stmt(astroot);
            break;
        }
        case ast_and_stmt:
        {
            traverse_ast_and_stmt(astroot);
            break;
        }
        case ast_eq_stmt:
        {
            traverse_ast_eq_stmt(astroot);
            break;
        }
        case ast_neq_stmt:
        {
            traverse_ast_neq_stmt(astroot);
            break;
        }
        case ast_lt_stmt:
        {
            traverse_ast_lt_stmt(astroot);
            break;
        }
        case ast_gt_stmt:
        {
            traverse_ast_gt_stmt(astroot);
            break;
        }
        case ast_geq_stmt:
        {
            traverse_ast_geq_stmt(astroot);
            break;
        }
        case ast_leq_stmt:
        {
            traverse_ast_leq_stmt(astroot);
            break;
        }
        case ast_add_stmt:
        {
            traverse_ast_add_stmt(astroot);
            break;
        }
        case ast_sub_stmt:
        {
            traverse_ast_sub_stmt(astroot);
            break;
        }
        case ast_mul_stmt:
        {
            traverse_ast_mul_stmt(astroot);
            break;
        }
        case ast_div_stmt:
        {
            traverse_ast_div_stmt(astroot);
            break;
        }
        case ast_unary_not:
        {
            traverse_ast_unary_not(astroot);
            break;
        }
        case ast_unary_add:
        {
            traverse_ast_unary_add(astroot);
            break;
        }
        case ast_unary_sub:
        {
            traverse_ast_unary_sub(astroot);
            break;
        }
        case ast_const_val:
        {
            traverse_ast_const_val(astroot);
            break;
        }
        case ast_print_stmt:
        {
            traverse_ast_print_stmt(astroot);
            break;
        }
        case ast_input_stmt:
        {
            traverse_ast_input_stmt(astroot);
            break;
        }
        case ast_return_stmt:
        {
            traverse_ast_return_stmt(astroot);
            break;
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
        printf("Types: %d    %d\n",astroot->child[0]->datatype,astroot->child[1]->datatype);
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
    if(search_symbol(func->name) != NULL){
        printf("Function with name %s already exists\n",func->name);
        exit(0);
    }
    Symbol* cur_param = NULL;
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
        // printf("Parameters: %s\n",params->child[1]->symbol->name);
        params = params->child[0];
    }
    push_symbol(func);// push the function symbol
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
    Symbol* func = search_symbol(astroot->symbol->name);
    if(func == NULL){
        printf("No function with name %s exists.\n",astroot->symbol->name);
        exit(0);
    }
    Symbol* params = func->param_list;
    while(args != NULL && params != NULL){
        if(!convertible_types(params->type,args->child[1]->datatype)){
            printf("Type mismatch in function %s for parameter %s\n",astroot->symbol->name,params->name);
            exit(0);
        }
        // printf("Checking: %d\n",args->child[1]->datatype);
        // printf("Checking : %s\n",params->name);
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
    // printf("Return type : %d\n",func->type);
    astroot->datatype = func->type;
}

int get_size(int type){
    if(type == INT_TYPE){
        return 4;
    }
    if(type == DOUBLE_TYPE){
        return 8;
    }
    if(type == STR_TYPE){
        return 300;
    }
    if(type == BOOL_TYPE){
        return 1;
    }
    return 0;
}

void update_counter() {
    global_counter++;
}

int get_register () {
    int min_index = 0;
    for (int i = 0; i < 18; i++) {
        if (lru_counter[i] < lru_counter[min_index])
            min_index = i;
    }
    lru_counter[min_index] = global_counter;
    update_counter();

    return min_index+8; // Offset as reg starts from 8
}

void update_fcounter(){
    global_fcounter++;
}

int get_fregister () {
    int min_index = 0;
    for (int i = 0; i < 18; i++) {
        if (lru_fcounter[i] < lru_fcounter[min_index])
            min_index = i;
    }
    lru_fcounter[min_index] = global_fcounter;
    update_fcounter();

    return min_index+8; // Offset as reg starts from 8
}

void update_register(int index) {
    lru_counter[index-8] = global_counter;
    update_counter();
}