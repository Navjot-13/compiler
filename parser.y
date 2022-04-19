%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

#include "Utils/symbol_table.h"
#include "Utils/ast.h"

SymbolTable* current_symbol_table;
SymbolTable* persistent_symbol_table;
AST* astroot;
int unused_scope;
int current_scope;

int yylex();
int yyerror(char *);
%}

%union {
  int int_val;
  double double_val;
  char str_val[300];
  struct AST *ast;
  struct Symbol *symbol;
}


%token ADD SUB MUL DIV ASSIGN EQ NEQ TRU FLS AND OR NOT INT DCML BOOL STR SCOL BGN AEQ MEQ SEQ DEQ INCR DECR GEQ LEQ ARR IF ELSE LP BRK RETURN CMNT MLTI_CMNT PRINT INPUT
%token <int_val> INT_CONST 
%token <double_val> DCML_CONST
%token <str_val> STR_CONST ID
%type<int_val> data_type comp_op
%type<ast> statements stmt_list stmt cond_stmt assign_stmt array_decl arr_variable program X func_list 
function params param_list param_type cond_stmt2 args arg_list expressions expr cond_or_stmt cond_and_stmt 
eql_stmt comp_stmt arithmetic_stmt1 arithmetic_stmt2 unary_op_stmt constant loop_stmt variable L print_stmt 
input_stmt
%%

program:        func_list  BGN statements 
                {
                        printf("No problem\n");
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        astroot = make_node(ast_start_stmt,push,$1,$3,pop);
                }
                ; 

/*----------------Function Declaration ----------------------*/
func_list:      func_list function
                {
                        $$ = make_node(ast_func_list_stmt,$1,$2,NULL,NULL);
                }

                |

                {
                        $$ = NULL;
                }
                
                ;

function:       data_type ID '(' params ')' '{' stmt_list '}'
                {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        $$ = make_node(ast_func_stmt,push,$4,$7,pop);
                        char *name = (char*)malloc((strlen($2)+1)*sizeof(char));
                        strcpy(name, $2);
                        $$->symbol = symbol_init(name,$1,NULL,NULL);
                        $$->symbol->is_function = 1;
                }

                ;

params:         param_list
                {
                        $$ = $1;
                }

                |

                {
                        $$ = NULL;
                }
                
                ;

param_list:     param_list',' param_type
                {
                        $$ = make_node(ast_param_list_stmt,$1,$3,NULL,NULL);
                }

                | param_type
                
                {
                        $$ = make_node(ast_param_list_stmt,NULL,$1,NULL,NULL);
                }
                
                ;

param_type:     data_type ID
                {
                        $$ = make_node(ast_param_stmt,NULL,NULL,NULL,NULL);
                        char *name = (char*)malloc((strlen($2)+1)*sizeof(char));
                        strcpy(name, $2);
                        $$->symbol = symbol_init(name,$1,NULL,NULL);
                }

                ;

args:           arg_list
                {
                        $$ = $1;
                }
                
                |

                {
                        $$ = NULL;
                }
                
                ;

arg_list:       arg_list',' expr
                {
                        $$ = make_node(ast_arg_list_stmt,$1,$3,NULL,NULL);
                }
                
                | expr
                
                {
                        $$ = make_node(ast_arg_list_stmt,NULL,$1,NULL,NULL);
                }

                ;        
/*------------------------------------------------------------*/



/*------------------Statement Declaration---------------------*/
statements:     '{' stmt_list '}' {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        $$ = make_node(ast_stmts,push,$2,pop,NULL);
                }
                
                |
                
                {
                        $$ = NULL;
                }
                ;

stmt_list:      stmt stmt_list 
                {
                        $$ = make_node(ast_stmt_list,$1,$2,NULL,NULL);
                }
                
                |  
                
                {
                        $$ = NULL;
                }
                ;

stmt:           assign_stmt 
                {
                        $$ = $1;
                }

                | print_stmt
                
                {
                        $$ = $1;
                }

                | input_stmt
                
                {
                        $$ = $1;
                }
                
                | cond_stmt 
                
                {
                        $$ = $1;
                } 
                
                | loop_stmt
                
                {
                        $$ = $1;
                } 
                
                | array_decl
                
                {
                        $$ = $1;
                } 
                
                | expressions 
                
                {
                        $$ = $1;
                }

                | RETURN expr SCOL

                {
                        $$ = make_node(ast_return_stmt,$2,NULL,NULL,NULL);
                }
                ;

print_stmt:     PRINT '(' expr ')' SCOL
                {
                        $$ = make_node(ast_print_stmt,$3,NULL,NULL,NULL);
                }
                ;


input_stmt:     variable ASSIGN INPUT '(' ')' SCOL
                {
                       $$ = make_node(ast_input_stmt,$1,NULL,NULL,NULL);
                }
                ;

assign_stmt:    data_type L SCOL 
                {
                        $$ = make_node(ast_decl_stmt,$2,NULL,NULL,NULL);
                        $2->datatype = $1;
                }
                ;

L:              L ',' ID 
                {       
                        AST* var = make_node(ast_variable_stmt,NULL,NULL,NULL,NULL);
                        char *name = (char*)malloc((strlen($3)+1)*sizeof(char));
                        strcpy(name, $3);
                        var->symbol = symbol_init(name,-1,NULL,NULL);
                        
                        $$ = make_node(ast_var_list,$1,var,NULL,NULL);
                }
                
                | 
                
                ID      
                {
                        $$ = make_node(ast_variable_stmt,NULL,NULL,NULL,NULL);
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,-1,NULL,NULL);
                }
                ;

array_decl:     ARR '<' X ',' INT_CONST'>' ID SCOL
                {
                        $$ = make_node(ast_array_decl_stmt,$3,NULL,NULL,NULL);
                        char* name = (char*)malloc((strlen($7)+1)*sizeof(char));
                        strcpy(name,$7);
                        $$->symbol = symbol_init(name,$3->datatype,NULL,NULL);
                        $$->symbol->is_array = 1;
                        if($3->size == -1){
                                $$->symbol->size = $5;
                        } else {
                                $$->symbol->size = $3->size * $5;
                        }
                }
                ;
X:              ARR '<' X ',' INT_CONST '>' 
                {
                        $$ = make_node(ast_array_stmt,$3,NULL,NULL,NULL);
                        if($3->size == -1){
                                $$->size = $5;
                        } else {
                                $$->size = $3->size * $5;
                        }
                        $$->datatype = $3->datatype;
                }

                | data_type
                
                {
                        $$ = make_node(ast_array_datatype_stmt,NULL,NULL,NULL,NULL);
                        $$->datatype = $1;
                }
                ;


expressions:    expr SCOL 
                {
                        $$ = $1;
                }
                ;

expr:           variable ASSIGN expr 
                {
                        $$ = make_node(ast_assgn_stmt,$1,$3,NULL,NULL);
                }
                
                | variable AEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* ast = make_node(ast_add_stmt,$1,$3,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,ast,NULL,NULL);
                }
                
                | variable SEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* ast = make_node(ast_sub_stmt,$1,$3,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,ast,NULL,NULL);
                }
                
                | variable MEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* ast = make_node(ast_mul_stmt,$1,$3,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,ast,NULL,NULL);
                }
                
                | variable DEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* ast = make_node(ast_div_stmt,$1,$3,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,ast,NULL,NULL);
                }
                
                | variable INCR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* const_ast = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        const_ast->val.int_val = 1;
                        const_ast->datatype = INT_TYPE;
                        AST* variable_ast = make_node(ast_add_stmt,$1,const_ast,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,variable_ast,NULL,NULL);
                }
                
                | variable DECR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        AST* const_ast = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        const_ast->val.int_val = 1;
                        const_ast->datatype = INT_TYPE;
                        AST* variable_ast = make_node(ast_sub_stmt,$1,const_ast,NULL,NULL);
                        $$ = make_node(ast_assgn_stmt,$1,variable_ast,NULL,NULL);
                }
                
                | cond_or_stmt 
                
                {
                        $$ = $1;
                }
                ;

cond_or_stmt:   cond_or_stmt OR cond_and_stmt 
                {
                        $$ = make_node(ast_or_stmt,$1,$3,NULL,NULL);
                }
                
                | cond_and_stmt 
                
                {
                        $$ = $1;
                }
                ;

cond_and_stmt:  cond_and_stmt AND eql_stmt 
                {
                        $$ = make_node(ast_and_stmt,$1,$3,NULL,NULL);
                }
                
                | eql_stmt 
                    
                {
                        $$ = $1;
                }
                ;

eql_stmt:       eql_stmt EQ comp_stmt 
                {
                        $$ = make_node(ast_eq_stmt,$1,$3,NULL,NULL);
                }
                
                | eql_stmt NEQ comp_stmt 
                
                {
                        $$ = make_node(ast_neq_stmt,$1,$3,NULL,NULL);
                }
                
                | comp_stmt 
                
                {
                        $$ = $1;
                }
                ;

comp_stmt:      comp_stmt comp_op arithmetic_stmt1 
                {
                        $$ = make_node($2,$1,$3,NULL,NULL);
                        $$->datatype = BOOL_TYPE;
                }
                
                | arithmetic_stmt1 
                
                {
                        $$ = $1;
                }
                ;

comp_op:        '>'     
                {
                        $$ = ast_gt_stmt;
                }
                    
                | '<'  
                
                {
                        $$ = ast_lt_stmt;
                }
                
                | GEQ  
                
                {
                        $$ = ast_geq_stmt;     
                }
                
                | LEQ  
                
                {
                        $$ = ast_leq_stmt;
                }
                ;

arithmetic_stmt1:       arithmetic_stmt1 ADD arithmetic_stmt2   
                        {
                                $$ = make_node(ast_add_stmt,$1,$3,NULL,NULL);
                        }
                        
                        | 
                        
                        arithmetic_stmt1 SUB arithmetic_stmt2 
                        {
                                $$ = make_node(ast_sub_stmt,$1,$3,NULL,NULL);
                        }
                        
                        | 
                        arithmetic_stmt2 
                        {
                                $$ = $1;
                        }
                    ;

arithmetic_stmt2:       arithmetic_stmt2 MUL unary_op_stmt 
                        {
                                $$ = make_node(ast_mul_stmt,$1,$3,NULL,NULL);
                        }
                        
                        | arithmetic_stmt2 DIV unary_op_stmt 
                        
                        {
                                $$ = make_node(ast_div_stmt,$1,$3,NULL,NULL);
                        }
                        
                        | unary_op_stmt 
                        
                        {
                                $$ = $1;
                        }
                    ;

unary_op_stmt:  NOT unary_op_stmt 
                {
                        $$ = make_node(ast_unary_not,$2,NULL,NULL,NULL);
                }
                
                | ADD unary_op_stmt 
                
                {
                        $$ = make_node(ast_unary_add,$2,NULL,NULL,NULL);
                }
                
                | SUB unary_op_stmt 
                
                {
                        $$ = make_node(ast_unary_sub,$2,NULL,NULL,NULL);
                }
                
                | variable 
                
                {
                        $$ = $1;
                }
                
                | constant 
                
                {    
                        $$ = $1;
                }
                
                | '(' expr ')'
                
                {
                        $$ = $2;
                }
                ;

variable:       ID 
                {
                        $$ = make_node(ast_var_expr,NULL,NULL,NULL,NULL);
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,-1,NULL,NULL);
                }
                
                | ID '(' args ')' 
                
                {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        $$ = make_node(ast_func_call_stmt,push,$3,pop,NULL);
                        char* name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name,$1);
                        $$->symbol = symbol_init(name,-1,NULL,NULL);
                        $$->symbol->is_function = 1;
                }
                
                | arr_variable 
                
                {
                        $$ = $1;
                }
                
                ;

arr_variable:   ID'['expr']'
                {
                        $$ = make_node(ast_arry_assgn_stmt,NULL,$3,NULL,NULL);
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,-1,NULL,NULL);
                }
                
                | arr_variable '['expr']'

                {
                        $$ = make_node(ast_arry_assgn_stmt,$1,$3,NULL,NULL);
                        $$->symbol = $1->symbol;
                }
                ;


cond_stmt:      IF '(' expr ')' '{' stmt_list '}' cond_stmt2
                {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        AST* ast_if = make_node(ast_if_stmt,$3,push,$6,pop);
                        $$ = make_node(ast_cond_stmt,ast_if,$8,NULL,NULL);
                }

                ;

cond_stmt2:     ELSE '{' stmt_list '}'
                {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        $$ = make_node(ast_cond_stmt,push,$3,pop,NULL);
                }
                
                | ELSE cond_stmt

                {
                        $$ = $2;
                }

                |

                {
                        $$ = NULL;
                }

                ;



loop_stmt:      LP '(' expr ')' '{' stmt_list '}'
                {
                        AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                        AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                        $$ = make_node(ast_loop_stmt,$3,$6,NULL,NULL);
                }
                ;


data_type:      INT 
                {
                        $$ = INT_TYPE;
                }

                | DCML
                
                {
                        $$ = DOUBLE_TYPE;
                }
                        
                | STR 
                
                {
                        $$ = STR_TYPE;
                }
                
                | BOOL 
                
                {
                        $$ = BOOL_TYPE;
                };

constant:       INT_CONST 
                {
                        $$ = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        $$->val.int_val = $1;
                        $$->datatype = INT_TYPE;
                }

                | DCML_CONST 

                {
                        $$ = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        $$->val.double_val = $1;
                        $$->datatype = DOUBLE_TYPE;
                }

                | STR_CONST

                {
                        $$ = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        strcpy($$->val.str_val,$1);
                        $$->datatype = STR_TYPE;
                }
                
                | TRU 
                
                {
                        $$ = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        $$->val.bool_val = true;
                        $$->datatype = BOOL_TYPE;
                }

                | FLS
                
                {
                        $$ = make_node(ast_const_val,NULL,NULL,NULL,NULL);
                        $$->val.bool_val = false;
                        $$->datatype = BOOL_TYPE;
                }
                ;
/*------------------------------------------------------------*/

%%

int yyerror(char *s){
  printf("Error: %s\n", s);
}
