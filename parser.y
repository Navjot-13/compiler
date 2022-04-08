%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

#include "Utils/symbol_table.h"
#include "Utils/ast.h"

Symbol *stack;
SymbolTable* symbol_table;
AST* astroot;

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
%type<ast> statements stmt_list stmt cond_stmt assign_stmt array_decl arr_variable program
expressions expr cond_or_stmt cond_and_stmt eql_stmt comp_stmt arithmetic_stmt1 arithmetic_stmt2 unary_op_stmt constant loop_stmt variable L print_stmt input_stmt printable
%%

program:        func_list  BGN statements 
                {
                        printf("No problem\n");
                        astroot = $3;
                }
                ; 

/*----------------Function Declaration ----------------------*/
func_list:          func_list function|  ;

function:           data_type ID '(' params ')' statements;

params:             param_list
                    | ;

param_list:         param_list',' param_type 
                    | param_type;

param_type:         data_type ID;

args:               arg_list
                    | ;

arg_list:           arg_list',' expr
                    | expr;        
/*------------------------------------------------------------*/



/*------------------Statement Declaration---------------------*/
statements:             '{' stmt_list '}' {
                                AST *push = make_node(ast_push_scope,NULL,NULL,NULL,NULL);
                                AST *pop = make_node(ast_pop_scope,NULL,NULL,NULL,NULL);
                                $$ = make_node(ast_stmts,push,$2,pop,NULL);
                        }
                    | 
                        { push_symbol_table(); }
                        stmt {
                                $$ = $2;
                                pop_symbol_table();
                        }
                    ;

stmt_list:      stmt_list stmt 
                {
                        $$ = make_node(ast_stmt_list,$1,$2,NULL,NULL);
                }
                
                | stmt  
                
                {
                        $$ = $1;
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
                ;

print_stmt:     PRINT '(' printable ')' SCOL
                {
                        $$ = make_node(ast_print_stmt,$3,NULL,NULL,NULL);
                }
                ;

printable:      expr
                {      
                        $$ = $1;
                }

                | variable
                {
                        $$ = $1;
                }

                | constant
                {
                        $$ = $1;
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
                        $$ = make_node(ast_array_decl_stmt,NULL,NULL,NULL,NULL);
                }
                ;
X:              ARR '<' X ',' INT_CONST '>' 
                {
                        
                }

                | data_type
                
                {
                        
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
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_aeq_stmt,$1,$3,NULL,NULL);
                }
                
                | variable SEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_seq_stmt,$1,$3,NULL,NULL);
                }
                
                | variable MEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_meq_stmt,$1,$3,NULL,NULL);
                }
                
                | variable DEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_deq_stmt,$1,$3,NULL,NULL);
                }
                
                | variable INCR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        $$ = make_node(ast_incr_stmt,$1,NULL,NULL,NULL);
                }
                
                | variable DECR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        $$ = make_node(ast_decr_stmt,$1,NULL,NULL,NULL);
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
                }
                
                | arithmetic_stmt1 
                
                {
                        $$ = $1;
                }
                ;

comp_op:        '>'     
                {
                        $$ = ast_lt_stmt;
                }
                    
                | '<'  
                
                {
                        $$ = ast_gt_stmt;
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
                ;

variable:       ID 
                {
                        // Symbol* symbol = search_symbol($1);
                        // if(symbol == NULL){
                        //         printf("Identifier undeclared : %s\n",$1);
                        //         return 0;
                        // }
                        $$ = make_node(ast_var_expr,NULL,NULL,NULL,NULL);
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,-1,NULL,NULL);
                }
                
                | ID '(' args ')' {}
                
                | arr_variable 
                
                {
                        
                }
                
                ;

arr_variable:   ID'['expr']'
                {
                        $$ = make_node(ast_arry_assgn_stmt,$3,NULL,NULL,NULL);
                        Symbol* symbol = search_symbol($1);
                        if(symbol == NULL){
                                printf("Identifier undeclared : %s\n",$1);
                                return 0;
                        }
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,symbol->type,NULL,NULL);   
                }
                
                | arr_variable '['expr']'

                {
                        $$ = make_node(ast_arry_assgn_stmt,$1,$3,NULL,NULL);
                        $$->symbol = $1->symbol;
                }
                ;


cond_stmt:      IF '(' expr ')' '{' 
                {
                        push_symbol_table();
                }
                stmt_list
                {
                        pop_symbol_table();
                }
                '}' cond_stmt2 {};

cond_stmt2:         ELSE
                        {
                                push_symbol_table();
                        }
                        stmt 
                        {
                                pop_symbol_table();
                        }
                        | ;



loop_stmt:          LP '(' expr ')' statements {    
                                $$ = make_node(ast_loop_stmt,$3,$5,NULL,NULL);
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
                
                | TRU {}
                | FLS {};
/*------------------------------------------------------------*/

%%

int yyerror(char *s){
  printf("Error: %s\n", s);
}
