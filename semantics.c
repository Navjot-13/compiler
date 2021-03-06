#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/ast.h"
#include "Utils/symbol_table.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;
extern SymbolTable* current_symbol_table;
FILE *fp;

int registers[18];      // Registers from $8 to $25
int lru_counter[18];
int global_counter = 1;
int fregisters[18];     // float registers
int lru_fcounter[18];
int global_fcounter = 1;
int global_offset = 0;
int label = 0;
int str_label = 0;  // label for pushloop
void assign_type (AST *astroot);
void traverse(AST *astroot);
void typecheck(AST *astroot);
void binary_op_type_checking(AST *astroot);
void add_params(AST* astroot);
void check_params(AST* astroot);
bool convertible_types(int type1,int type2);
void generate_code(AST* astroot);
int get_size(int type);
void update_counter();
void update_register(int index);
void update_fregister(int index);
int get_register();
int get_fregister();
void push_registers_on_stack();
void pop_registers_from_stack();

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
    
    fp = fopen("assembly.asm","w+");
    fprintf(fp,"    .data\n");
    fprintf(fp,"        str_buffer: .space 300\n");
    fprintf(fp,"    .text\n");
    fprintf(fp,"    .globl main\n");
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
    fprintf(fp,"main:\n");
    // fprintf(fp,"    la $sp, -8($sp)\n"); // allocate space for old $fp and $ra
    // fprintf(fp,"    sw $fp, 4($sp)\n"); // save old fp
    // fprintf(fp,"    sw $ra, 0($sp)\n");// save return address
    fprintf(fp,"    la $fp, 0($sp)\n");// set up frame pointer
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
    for(int i = 0; i < 4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_func_stmt(AST* astroot)
{
    fprintf(fp,"__%s__:\n",astroot->symbol->name);
    int temp = global_offset;
    global_offset = 4;
    add_params(astroot);
    traverse(astroot->child[0]);
    traverse(astroot->child[1]);
    astroot->return_type = astroot->symbol->type;
    // assign the return type of the function
    astroot->child[2]->return_type = astroot->return_type;
    traverse(astroot->child[2]);
    traverse(astroot->child[3]);
    fprintf(fp,"    jr $ra\n"); // return
    global_offset = temp;
}

void traverse_func_list_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
}

void traverse_ast_param_list_stmt(AST* astroot)
{
    // printf("OFFSET: %d\n",astroot->child[1]->symbol->offset);
    // astroot->child[1]->symbol->offset -= 8;
    push_symbol(astroot->child[1]->symbol);
    traverse(astroot->child[1]);
    // fprintf(fp,"    la $sp, -%d($sp)\n",get_size(astroot->child[1]->symbol->type));// allocate stack frame
    traverse(astroot->child[0]);
}

void traverse_ast_param_stmt(AST* astroot)
{
    astroot->symbol->size = get_size(astroot->symbol->type);
    // printf("PARAM NAME: %s   OFFSET: %d\n",astroot->symbol->name,astroot->symbol->offset);
    push_symbol(astroot->symbol);
}

void traverse_ast_arg_list_stmt(AST* astroot)
{
    int add_to_stack = astroot->child[1]->symbol->offset + 8;
    // printf("OFFSET: %d\n",astroot->child[1]->symbol->offset);
    // astroot->child[1]->symbol->offset -= 8;
    push_symbol(astroot->child[1]->symbol);
    current_symbol_table = current_symbol_table->prev;
    traverse(astroot->child[1]);
    current_symbol_table = current_symbol_table->next;
    // fprintf(fp,"    la $sp, -%d($sp)\n",get_size(astroot->child[1]->symbol->type));// allocate stack frame
    fprintf(fp,"    addi $sp,$sp, -%d\n",add_to_stack);
    fprintf(fp,"    sw $%d 0($sp)\n",astroot->child[1]->reg);
    fprintf(fp,"    addi $sp,$sp, %d\n",add_to_stack);
    traverse(astroot->child[0]);
}

void traverse_ast_func_call_stmt(AST* astroot)
{
    int temp = global_offset;
    global_offset = 4;
    check_params(astroot);
    // printf("Offset: %d\n",astroot->child[1]->child[1]->symbol->offset);
    traverse(astroot->child[0]);
    traverse(astroot->child[1]);
    fprintf(fp,"    la $sp, -8($sp)\n"); // allocate space for old $fp and $ra
    fprintf(fp,"    sw $fp, 4($sp)\n"); // save old fp
    fprintf(fp,"    sw $ra, 0($sp)\n");// save return address
    fprintf(fp,"    la $fp, 0($sp)\n");// set up frame pointer
    if(global_offset > 4){
        fprintf(fp,"    addi $sp, $sp, -%d\n",global_offset - 4);// shift stack pointer upto parameters size
    }
    push_registers_on_stack();
    fprintf(fp,"    jal   __%s__\n",astroot->symbol->name);// jump to the function label
    pop_registers_from_stack();
    if(global_offset > 4){
        fprintf(fp,"    addi $sp, $sp, %d\n",global_offset - 4);// shift stack pointer upto parameters size
    }
    traverse(astroot->child[2]);
    fprintf(fp,"    la $sp, 0($fp)\n");// deallocate labels
    fprintf(fp,"    lw $ra, 0($sp)\n");// restore return address
    fprintf(fp,"    lw $fp, 4($sp)\n");// restore frame pointer
    fprintf(fp,"    la $sp, 8($sp)\n");// restore stack pointer
    global_offset = temp;
    if(astroot->datatype == INT_TYPE || astroot->datatype == BOOL_TYPE){
        astroot->reg = 2;
    } else if(astroot->datatype == STR_TYPE){

    } else {

    } 

}

void traverse_ast_stmt_list(AST* astroot)
{
    astroot->child[0]->next = label++;
    astroot->child[0]->return_type = astroot->return_type;
    if(astroot->child[1]){
        astroot->child[1]->next = astroot->next;
        astroot->child[1]->return_type = astroot->return_type;
    }
    traverse(astroot->child[0]);
    if(astroot->child[0]->type == ast_cond_stmt || astroot->child[0]->type == ast_loop_stmt){
        fprintf(fp,"__%d__:\n",astroot->child[0]->next);
    }
    traverse(astroot->child[1]);
}

void traverse_ast_assgn_stmt(AST* astroot)
{
    for(int i = 0; i < 4;++i){
        traverse(astroot->child[i]);
    }
    // printf("Type 1: %d Type 2: %d\n",astroot->child[0]->type,astroot->child[1]->type);
    // printf("Datatype of left child: %d\n",astroot->child[0]->datatype);
    astroot->val = astroot->child[1]->val;
    astroot->datatype = astroot->child[0]->datatype;  
    typecheck(astroot);
    // Generate Code (Considering only integers for now)

    printf("asdfasdfasdfas %d\n",astroot->child[0]->symbol->is_array);

    if(astroot->child[0]->symbol->is_array == 1){
        if(astroot->child[0]->datatype == INT_TYPE){
            // int reg1 = get_reg();
            // int reg2 = get_reg();
            // fprintf(fp, "    li $%d, %d\n", reg1, local_offset);
            // fprintf(fp, "    sub $%d, $fp, $%d\n", reg2, reg1);
            // fprintf(fp, "    sw $%d, 0($%d)\n")
            // astroot->reg = reg;

            int reg2 = astroot->child[0]->child[1]->reg;
            int reg1 = get_register();
            int reg3 = get_register();
            
            fprintf(fp, "    sub $%d, $fp, %d\n", reg1, astroot->child[0]->symbol->offset);
            fprintf(fp, "    mul $%d, $%d, 4\n", reg3, reg2);
            fprintf(fp, "    add $%d, $%d, $%d\n", reg1, reg1, reg3);
            fprintf(fp, "    sw $%d, 0($%d)\n", astroot->child[1]->reg, reg1);

            astroot->reg = astroot->child[1]->reg;
        }
    }

    else if(astroot->datatype == INT_TYPE || astroot->datatype == BOOL_TYPE){
        printf("%s is the register with offset %d\n", astroot->child[0]->symbol->name, astroot->child[0]->symbol->offset);
        fprintf(fp, "    sw $%d, -%d($fp)\n", astroot->child[1]->reg, astroot->child[0]->symbol->offset);
        astroot->reg = astroot->child[1]->reg;
    }

    else if(astroot->datatype == DOUBLE_TYPE){
        // printf("%s is the register with offset %d\n", astroot->child[0]->symbol->name, astroot->child[0]->symbol->offset);
        fprintf(fp, "    s.s $f%d, -%d($fp)\n", astroot->child[1]->freg, astroot->child[0]->symbol->offset);
        astroot->freg = astroot->child[1]->freg;
    }

    else if(astroot->datatype == STR_TYPE){
        int reg1 = get_register();
        int reg2 = get_register();
        int reg3 = get_register();
        int reg4 = get_register();

        update_register(reg1);
        update_register(reg2);
        update_register(reg3);
        update_register(reg4);

    //     __loop__:
    // lb $t3, 0($t1);
    // sb $t3, 0($t2);
    // addi $t1, $t1, 1;
    // addi $t2, $t2, 1;
    // bne $t3, $zero, __loop__;

        fprintf(fp, "    la $%d, str_buffer\n", reg1);
        fprintf(fp, "    li $%d, %d\n", reg3, astroot->child[0]->symbol->offset);
        fprintf(fp, "    sub $%d, $fp, $%d\n", reg2, reg3);
        fprintf(fp,"     __pushloop%d__:", str_label); 
        fprintf(fp, "       lb $%d, 0($%d)\n", reg4, reg1);  
        fprintf(fp, "       sb $%d, ($%d)\n", reg4, reg2);
        fprintf(fp, "       addi $%d, $%d, 1\n", reg1, reg1);
        fprintf(fp, "       addi $%d, $%d, 1\n", reg2, reg2);
        fprintf(fp, "       bne $%d, $zero, __pushloop%d__\n", reg4, str_label);

        str_label++;
        
        // astroot->reg
    }

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
            fprintf(fp,"    beqz $%d __%d__\n",astroot->child[0]->child[0]->reg,astroot->child[0]->child[0]->fal);
            // fprintf(fp,"   j __%d__\n",astroot->child[0]->child[0]->fal);
            // fprintf(fp,"__%d__:\n",astroot->child[0]->child[0]->tru);
            astroot->child[0]->child[2]->return_type = astroot->return_type;
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
            fprintf(fp,"    beqz $%d __%d__\n",astroot->child[0]->child[0]->reg,astroot->child[0]->child[0]->fal);
            // fprintf(fp,"   j __%d__\n",astroot->child[0]->child[0]->fal);
            // fprintf(fp,"__%d__:\n",astroot->child[0]->child[0]->tru);
            astroot->child[0]->child[2]->return_type = astroot->return_type;
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
    // if (astroot->symbol == NULL)
    //     printf("check\n");
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->symbol->size = get_size(astroot->symbol->type) * astroot->symbol->size;
    printf("Array size: %d\n",astroot->symbol->size);

    global_offset += astroot->symbol->size;
    astroot->symbol->offset = global_offset;
    push_symbol(astroot->symbol);
    

    // generate code
    fprintf(fp, "    la $sp, -%d($fp)\n", astroot->symbol->size);
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
    astroot->symbol = symbol;
    astroot->datatype = symbol->type;

    int reg2 = astroot->child[1]->reg;

    // int reg2 = astroot->child[0]->child[1]->reg;
    int reg1 = get_register();
    int reg3 = get_register();
    
    // printf("offset: %d\n",astroot->symbol->offset);
    fprintf(fp, "    sub $%d, $fp, %d\n", reg1, astroot->symbol->offset);
    fprintf(fp, "    mul $%d, $%d, 4\n", reg3, reg2);
    fprintf(fp, "    add $%d, $%d, $%d\n", reg1, reg1, reg3);
    fprintf(fp, "    lw $%d, 0($%d)\n", reg1, reg1);

    
    // fprintf(fp, "    mul $%d, $%d, 4\n", reg, reg);
    // fprintf(fp, "    addi $%d, $%d, %d\n", reg, reg, symbol->offset);
    // fprintf(fp, "    lw $%d, 0($%d)\n", reg, reg);

    astroot->reg = reg1;

    update_register(reg1);
    // update_register(reg2);
    

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
    global_offset += astroot->symbol->size;
    astroot->symbol->offset = global_offset;
    
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
    Symbol *symbol = search_symbol(astroot->symbol->name);
    if (symbol == NULL)
    {
        printf("\nError: Variable %s not declared\n", astroot->symbol->name);
        exit(0);
    }
    astroot->symbol = symbol;
    astroot->datatype = symbol->type;

    // Generate code

    if(astroot->datatype == DOUBLE_TYPE){
        int freg = get_fregister();
        astroot->freg = freg;
        fprintf(fp, "    l.s $f%d, -%d($fp)\n", freg, astroot->symbol->offset); 
        update_register(freg);
    }
    if(astroot->datatype == STR_TYPE){
        int reg = get_register();
        astroot->reg = reg;
        fprintf(fp, "    addi $%d, $fp, 0\n", reg);
        fprintf(fp, "    addi $%d, $%d, -%d\n", reg, reg, astroot->symbol->offset);
        update_register(reg);
    }
    // if(astroot->datatype == ARRAY_TYPE){
    //     int reg = get_register();
    //     astroot->reg = reg;
    //     fprintf(fp, "    addi $%d, $fp, 0\n", reg);
    //     fprintf(fp, "    addi $%d, $%d, -%d\n", reg, reg, astroot->symbol->offset);
    //     update_register(reg);
    // }
    else{
        int reg = get_register();
        astroot->reg = reg;
        fprintf(fp, "    lw $%d, -%d($fp)\n", reg, astroot->symbol->offset);
        update_register(reg);
    }

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
    fprintf(fp, "    sge $%d, $%d, $%d\n", reg0, reg0, reg1);
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
    binary_op_type_checking(astroot);
    
    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        int reg1 = astroot->child[1]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    add $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
        update_register(reg0);
        update_register(reg1);
    }
    // for floats
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        int freg1 = astroot->child[1]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    add.s $f%d, $f%d, $f%d\n", astroot->freg, freg0, freg1);
        update_fregister(freg0);
        update_fregister(freg1);
    }
}

void traverse_ast_sub_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        int reg1 = astroot->child[1]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    sub $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
        update_register(reg0);
        update_register(reg1);
    }

    //for floats
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        int freg1 = astroot->child[1]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    sub.s $f%d, $f%d, $f%d\n", astroot->freg, freg0, freg1);
        update_fregister(freg0);
        update_fregister(freg1);
    }
}

void traverse_ast_mul_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        int reg1 = astroot->child[1]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    mul $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
        update_register(reg0);
        update_register(reg1);
    }

    // floats
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        int freg1 = astroot->child[1]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    mul.s $f%d, $f%d, $f%d\n", astroot->freg, freg0, freg1);
        update_fregister(freg0);
        update_fregister(freg1);
    }
}

void traverse_ast_div_stmt(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    binary_op_type_checking(astroot);

    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        int reg1 = astroot->child[1]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    div $%d, $%d, $%d\n", astroot->reg, reg0, reg1);
        update_register(reg0);
        update_register(reg1);
    }

    // for float
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        int freg1 = astroot->child[1]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    div.s $f%d, $f%d, $f%d\n", astroot->freg, freg0, freg1);
        update_fregister(freg0);
        update_fregister(freg1);
    }
}

void traverse_ast_unary_not(AST* astroot)
{
    astroot->child[0]->tru = astroot->fal;
    astroot->child[0]->fal = astroot->tru;
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;

    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE || astroot->datatype == BOOL_TYPE){
        int reg0 = astroot->child[0]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    xori $%d, $%d, 1\n", astroot->reg, reg0);
        update_register(reg0);
    }
}

void traverse_ast_unary_add(AST* astroot)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;

    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    addi $%d, $%d, 0\n", astroot->reg, reg0);
        update_register(reg0);
    }

    // for floats
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    addi.s $f%d, $f%d, 0.0\n", astroot->freg, freg0);
        update_fregister(freg0);
    }
}

void traverse_ast_unary_sub(AST* root)
{
    for(int i = 0; i <4;++i){
        traverse(astroot->child[i]);
    }
    astroot->datatype = astroot->child[0]->datatype;
    
    // Generate Code (Considering only integers for now)
    if(astroot->datatype == INT_TYPE){
        int reg0 = astroot->child[0]->reg;
        astroot->reg = reg0;
        fprintf(fp, "    sub $%d, $0, $%d\n", astroot->reg, reg0);
        update_register(reg0);
    }

    // for floats
    if(astroot->datatype == DOUBLE_TYPE){
        int freg0 = astroot->child[0]->freg;
        astroot->freg = freg0;
        fprintf(fp, "    sub.s $f%d, $f0, $f%d\n", astroot->freg, freg0);
        update_fregister(freg0);
    }
    
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

    //for float
    if(astroot->datatype == DOUBLE_TYPE){
        int freg1 = get_fregister();
        astroot->freg = freg1;
        fprintf(fp, "   li.s $f%d, %lf\n", freg1, astroot->val.double_val);
    }

    //for bool
    if(astroot->datatype == BOOL_TYPE){
        int reg1 = get_register();
        astroot->reg = reg1;
        fprintf(fp, "    li $%d, %d\n", reg1, astroot->val.bool_val); 
    }

    if(astroot->datatype == STR_TYPE){

        int reg1 = get_register();
        // astroot->reg = reg1;
        int reg2 = get_register();

        update_register(reg1);
        update_register(reg2);

        int str_len = strlen(astroot->val.str_val);

        fprintf(fp, "    la $%d, str_buffer\n", reg1);
        astroot->val.str_val[str_len - 1] = '\0'; 
        for(int i = 1; i < str_len; i++){
            // if(i == str_len - 1){
            //     astroot->val.str_val[i] = '\0';
            // }
            fprintf(fp, "    li $%d, '%c'\n", reg2, astroot->val.str_val[i]);
            fprintf(fp, "    sb $%d, 0($%d)\n", reg2, reg1);
            fprintf(fp, "    addi $%d, $%d, 1\n", reg1, reg1);
        }
        
    }
    
}

void traverse_ast_print_stmt(AST* astroot)
{
    traverse(astroot->child[0]);

    if (astroot->child[0]->datatype == INT_TYPE || astroot->child[0]->datatype == BOOL_TYPE){
        int reg1 = astroot->child[0]->reg;
        update_register(reg1);
        fprintf(fp, "    li $v0, 1\n");
        fprintf(fp, "    move $a0, $%d\n", reg1);
        fprintf(fp, "    syscall\n");
        // print new line
        fprintf(fp, "    li $v0, 11\n");
        fprintf(fp, "    li $a0, 10\n");
        fprintf(fp, "    syscall\n");
    }
    
    if (astroot->child[0]->datatype == DOUBLE_TYPE){
        int freg1 = astroot->child[0]->freg;
        update_register(freg1);
        fprintf(fp, "    li $v0, 2\n");
        fprintf(fp, "    mov.s $f12, $f%d\n", freg1);
        fprintf(fp, "    syscall\n");
        // print new line
        fprintf(fp, "    li $v0, 11\n");
        fprintf(fp, "    li $a0, 10\n");
        fprintf(fp, "    syscall\n");
    }

    if(astroot->child[0]->datatype == STR_TYPE){
        int reg1 = astroot->child[0]->reg;
        update_register(reg1);
        fprintf(fp, "    li $v0, 4\n");
        fprintf(fp, "    move $a0, $%d\n", reg1);
        fprintf(fp, "    syscall\n");
        // print new line
        fprintf(fp, "    li $v0, 11\n");
        fprintf(fp, "    li $a0, 10\n");
        fprintf(fp, "    syscall\n");
    }
}

void traverse_ast_input_stmt(AST* astroot)
{
    Symbol *symbol = search_symbol(astroot->child[0]->symbol->name);
    if(symbol == NULL){
        printf("Identifier undeclared\n");
        exit(0);
    }

    traverse(astroot->child[0]);

    // For integer
    if (symbol->type == INT_TYPE || symbol->type == BOOL_TYPE) {
        fprintf(fp, "    li $v0, 5\n");
        fprintf(fp, "    syscall\n");

        int reg1 = get_register();
        astroot->reg = reg1;
        fprintf(fp, "    move $%d, $v0\n", reg1);
        fprintf(fp, "    sw $%d, -%d($fp)\n", reg1, astroot->child[0]->symbol->offset);
        update_register(reg1);
    }

    // For float
    if (symbol->type == DOUBLE_TYPE) {
        fprintf(fp, "    li $v0, 6\n");
        fprintf(fp, "    syscall\n");

        int freg1 = get_fregister();
        astroot->freg = freg1;
        fprintf(fp, "    mov.s $f%d, $f0\n", freg1);
        fprintf(fp, "    s.s $f%d, -%d($fp)\n", freg1, astroot->child[0]->symbol->offset);
        update_fregister(freg1);
    }

    //for string
    if(symbol->type == STR_TYPE){
        fprintf(fp, "    li $v0, 8\n");
        fprintf(fp, "    la $a0, str_buffer\n");
        fprintf(fp, "    li $a1, 300\n");
        fprintf(fp, "    syscall\n");

        int reg1 = get_register();
        int reg2 = get_register();
        int reg3 = get_register();
        int reg4 = get_register();

        update_register(reg1);
        update_register(reg2);
        update_register(reg3);
        update_register(reg4);


        fprintf(fp, "    la $%d, str_buffer\n", reg1);
        fprintf(fp, "    li $%d, %d\n", reg3, astroot->child[0]->symbol->offset);
        fprintf(fp, "       sub $%d, $fp, $%d\n", reg2, reg3);
        fprintf(fp,"     __pushloop%d__:", str_label); 
        fprintf(fp, "       lb $%d, 0($%d)\n", reg4, reg1);  
        fprintf(fp, "       sb $%d, ($%d)\n", reg4, reg2);
        fprintf(fp, "       addi $%d, $%d, 1\n", reg1, reg1);
        fprintf(fp, "       addi $%d, $%d, 1\n", reg2, reg2);
        fprintf(fp, "       bne $%d, $zero, __pushloop%d__\n", reg4, str_label);

        str_label++;
    }


}

void traverse_ast_return_stmt(AST* astroot)
{
    traverse(astroot->child[0]);
    if(astroot->return_type == -1){
        printf("Error: Return statement in begin function not allowed\n");
        exit(0);
    }
    if(!convertible_types(astroot->child[0]->datatype,astroot->return_type)){
        printf("Error: Return type of function doesn't match with the return statement type\n");
        exit(0);
    }
    if(astroot->return_type == INT_TYPE || astroot->return_type == BOOL_TYPE){
        fprintf(fp,"    add $v0 $0 $%d\n",astroot->child[0]->reg);
    } else if(astroot->return_type == DOUBLE_TYPE){

    } else if(astroot->return_type == STR_TYPE){

    }
    fprintf(fp,"    jr $ra\n"); // return

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
    printf("Type : %d\n",astroot->type);
    if (astroot->child[0]->datatype == INT_TYPE && astroot->child[1]->datatype == DOUBLE_TYPE) {
        astroot->child[1]->val.int_val= (int)astroot->child[1]->val.double_val;
        astroot->datatype = INT_TYPE;
        astroot->child[1]->datatype = INT_TYPE;

        // Assembly code to convert to integer
        int reg = get_register();
        fprintf(fp, "    cvt.w.s $f%d, $f%d\n", astroot->child[1]->freg, astroot->child[1]->freg);
        fprintf(fp, "    mfc1 $%d, $f%d\n", reg, astroot->child[1]->freg);    
        astroot->child[1]->reg = reg;

    } else if (astroot->child[0]->datatype == DOUBLE_TYPE && astroot->child[1]->datatype == INT_TYPE) {
        astroot->child[1]->val.double_val= (double)astroot->child[1]->val.int_val;
        astroot->datatype = DOUBLE_TYPE;
        astroot->child[1]->datatype = DOUBLE_TYPE;

        // Assembly code to convert to double
        int freg = get_fregister();
        fprintf(fp, "    mtc1 $%d, $f%d\n", astroot->child[1]->reg, freg);
        fprintf(fp, "    cvt.s.w $f%d, $f%d\n", freg, freg);
        astroot->child[1]->freg = freg;

    } else if (astroot->child[0]->datatype == INT_TYPE && astroot->child[1]->datatype == BOOL_TYPE) {
        astroot->child[1]->val.int_val= (int)astroot->child[1]->val.bool_val;
        astroot->datatype = INT_TYPE;
    } else if (astroot->child[0]->datatype == BOOL_TYPE && astroot->child[1]->datatype == INT_TYPE) {
        astroot->child[1]->val.bool_val= (bool)astroot->child[1]->val.int_val;
        astroot->datatype = BOOL_TYPE;
    } else if (astroot->child[0]->datatype == DOUBLE_TYPE && astroot->child[1]->datatype == BOOL_TYPE) {
        astroot->child[1]->val.double_val= (double)astroot->child[1]->val.bool_val;
        astroot->datatype = DOUBLE_TYPE;
    } else if (astroot->child[0]->datatype == BOOL_TYPE && astroot->child[1]->datatype == DOUBLE_TYPE) {
        astroot->child[1]->val.bool_val= (bool)astroot->child[1]->val.double_val;
        astroot->datatype = BOOL_TYPE;
    } 
    else if(astroot->child[0]->datatype != astroot->child[1]->datatype) {
        printf("Types: %d    %d\n",astroot->child[0]->datatype,astroot->child[1]->datatype);
        printf("\nError: Type mismatch in assignment statement\n");
        exit(0);
    }
}

void binary_op_type_checking(AST *astroot){
    if(astroot->child[0]->datatype == STR_TYPE || astroot->child[1]->datatype == STR_TYPE){
        printf("Invalid operands for +\n");
        exit(0);
    }
    if(astroot->child[0]->datatype == DOUBLE_TYPE || astroot->child[1]->datatype == DOUBLE_TYPE){
        astroot->datatype = DOUBLE_TYPE;

        if (astroot->child[1]->datatype == INT_TYPE || astroot->child[1]->datatype == BOOL_TYPE) {
            astroot->child[1]->datatype = DOUBLE_TYPE;
            astroot->child[1]->val.double_val= (double)astroot->child[1]->val.int_val;
            int freg = get_fregister();
            fprintf(fp, "    mtc1 $%d, $f%d\n", astroot->child[1]->reg, freg);
            fprintf(fp, "    cvt.s.w $f%d, $f%d\n", freg, freg);
            astroot->child[1]->freg = freg;
        }

        if (astroot->child[0]->datatype == INT_TYPE || astroot->child[0]->datatype == BOOL_TYPE) {
            astroot->child[0]->datatype = DOUBLE_TYPE;
            astroot->child[0]->val.double_val= (double)astroot->child[0]->val.int_val;
            int freg = get_fregister();
            fprintf(fp, "    mtc1 $%d, $f%d\n", astroot->child[0]->reg, freg);
            fprintf(fp, "    cvt.s.w $f%d, $f%d\n", freg, freg);
            astroot->child[0]->freg = freg;
        }

    } else if(astroot->child[0]->datatype == INT_TYPE || astroot->child[1]->datatype == INT_TYPE){
        astroot->datatype = INT_TYPE;
    } else{
        astroot->datatype = BOOL_TYPE;
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
        params->child[1]->symbol->size = get_size(params->child[1]->symbol->type);
        params->child[1]->symbol->offset = global_offset;
        global_offset += params->child[1]->symbol->size;
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
    AST* args = astroot->child[1];
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
        params->size = get_size(params->type);
        params->offset = global_offset;
        global_offset += params->size;
        args->child[1]->symbol = params;
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
        return 4;
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

void update_fregister(int index){
    lru_fcounter[index-8] = global_fcounter;
    update_fcounter();
}

void update_register(int index) {
    lru_counter[index-8] = global_counter;
    update_counter();
}

void push_registers_on_stack(){
    fprintf(fp,"    addi $sp, $sp, -72\n");
    for(int i = 8; i <= 25;++i){
        fprintf(fp,"    sw $%d, %d($sp)\n",i,(i-8)*4);
    }
}

void pop_registers_from_stack(){
    for(int i = 8; i <= 25;++i){
        fprintf(fp,"    lw $%d, %d($sp)\n",i,(i-8)*4);
    }
    fprintf(fp,"    addi $sp, $sp, 72\n");
}