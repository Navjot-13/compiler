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

const int INT_TYPE = 0;
const int DOUBLE_TYPE = 1;
const int STR_TYPE = 2;
const int BOOL_TYPE = 3;
const int ARRAY_TYPE = 4;
%}

%union {
  int int_val;
  double double_val;
  char str_val[300];
  struct AST *ast;
  struct Symbol *symbol;
}


%token ADD SUB MUL DIV ASSIGN EQ NEQ TRU FLS AND OR NOT INT DCML BOOL STR SCOL BGN AEQ MEQ SEQ DEQ INCR DECR GEQ LEQ ARR IF ELSE LP BRK RETURN CMNT MLTI_CMNT INP
%token <int_val> INT_CONST 
%token <double_val> DCML_CONST
%token <str_val> STR_CONST ID
%type<int_val> data_type comp_op
%type<symbol> L
%type<ast> statements stmt_list stmt cond_stmt assign_stmt array_decl arr_variable program
expressions expr cond_or_stmt cond_and_stmt eql_stmt comp_stmt arithmetic_stmt1 arithmetic_stmt2 unary_op_stmt constant loop_stmt variable
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
statements:             { push_symbol_table(); }
                        '{' stmt_list '}' {
                               $$ = $3;
                               pop_symbol_table();
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
                        $$ = make_node(ast_stmt_list,$1,$2);
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


assign_stmt:    data_type L SCOL 
                {
                        $$ = make_node(ast_decl_stmt,NULL,NULL);
                        while(stack != NULL){
                                Symbol* symbol = pop();
                                symbol->type = $1;
                                push_symbol(symbol);
                        }
                }
                ;

L:              L ',' ID 
                {       
                        $$ = symbol_init($3,-1,NULL,NULL);
                        push($$);
                }
                
                | 
                
                ID      
                {
                        $$ = symbol_init($1,-1,NULL,NULL);
                        push($$);
                }
                ;

array_decl:     ARR '<' X ',' INT_CONST'>' ID SCOL
                {
                        $$ = make_node(ast_array_decl_stmt,NULL,NULL);
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
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_assgn_stmt,$1,$3);
                }
                
                | variable AEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_aeq_stmt,$1,$3);
                }
                
                | variable SEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_seq_stmt,$1,$3);
                }
                
                | variable MEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_meq_stmt,$1,$3);
                }
                
                | variable DEQ expr 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        // if(symbol->type != $3->datatype){
                        //         printf("Type mismatch occurred.");
                        //         return 0;
                        // }
                        $$ = make_node(ast_deq_stmt,$1,$3);
                }
                
                | variable INCR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        $$ = make_node(ast_incr_stmt,$1,NULL);
                }
                
                | variable DECR 
                
                {
                        Symbol *symbol = search_symbol($1->symbol->name);
                        $$ = make_node(ast_decr_stmt,$1,NULL);
                }
                
                | cond_or_stmt 
                
                {
                        $$ = $1;
                }
                ;

cond_or_stmt:   cond_or_stmt OR cond_and_stmt 
                {
                        $$ = make_node(ast_or_stmt,$1,$3);
                }
                
                | cond_and_stmt 
                
                {
                        $$ = $1;
                }
                ;

cond_and_stmt:  cond_and_stmt AND eql_stmt 
                {
                        $$ = make_node(ast_and_stmt,$1,$3);
                }
                
                | eql_stmt 
                    
                {
                        $$ = $1;
                }
                ;

eql_stmt:       eql_stmt EQ comp_stmt 
                {
                        $$ = make_node(ast_eq_stmt,$1,$3);
                }
                
                | eql_stmt NEQ comp_stmt 
                
                {
                        $$ = make_node(ast_neq_stmt,$1,$3);
                }
                
                | comp_stmt 
                
                {
                        $$ = $1;
                }
                ;

comp_stmt:      comp_stmt comp_op arithmetic_stmt1 
                {
                        $$ = make_node($2,$1,$3);
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
                                $$ = make_node(ast_add_stmt,$1,$3);
                        }
                        
                        | 
                        
                        arithmetic_stmt1 SUB arithmetic_stmt2 
                        {
                                $$ = make_node(ast_sub_stmt,$1,$3);
                        }
                        
                        | 
                        arithmetic_stmt2 
                        {
                                $$ = $1;
                        }
                    ;

arithmetic_stmt2:       arithmetic_stmt2 MUL unary_op_stmt 
                        {
                                $$ = make_node(ast_mul_stmt,$1,$3);
                        }
                        
                        | arithmetic_stmt2 DIV unary_op_stmt 
                        
                        {
                                $$ = make_node(ast_div_stmt,$1,$3);
                        }
                        
                        | unary_op_stmt 
                        
                        {
                                $$ = $1;
                        }
                    ;

unary_op_stmt:  NOT unary_op_stmt 
                {
                        $$ = make_node(ast_unary_not,$2,NULL);
                }
                
                | ADD unary_op_stmt 
                
                {
                        $$ = make_node(ast_unary_add,$2,NULL);
                }
                
                | SUB unary_op_stmt 
                
                {
                        $$ = make_node(ast_unary_sub,$2,NULL);
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
                        Symbol* symbol = search_symbol($1);
                        if(symbol == NULL){
                                printf("Identifier undeclared : %s\n",$1);
                                return 0;
                        }
                        $$ = make_node(ast_variable_stmt,NULL,NULL);
                        char *name = (char*)malloc((strlen($1)+1)*sizeof(char));
                        strcpy(name, $1);
                        $$->symbol = symbol_init(name,symbol->type,NULL,NULL);
                }
                
                | ID '(' args ')' {}
                
                | arr_variable 
                
                {
                        
                }
                
                ;

arr_variable:   ID'['expr']'
                {
                        $$ = make_node(ast_arry_assgn_stmt,$3,NULL);
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
                        $$ = make_node(ast_arry_assgn_stmt,$1,$3);
                        $$->symbol = $1->symbol;
                }
                ;


cond_stmt:      IF '(' expr')' '{' 
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
                                $$ = make_node(ast_loop_stmt,$3,$5);
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
                        $$ = (AST*) malloc(sizeof(AST));
                        $$->val.int_val = $1;
                }

                | DCML_CONST 

                {
                        $$ = (AST*) malloc(sizeof(AST));
                        $$->val.double_val = $1;
                }

                | STR_CONST

                {
                        $$ = (AST*) malloc(sizeof(AST));
                        strcpy($$->val.str_val,$1);
                }
                
                | TRU {}
                | FLS {};
/*------------------------------------------------------------*/

%%

int yyerror(char *s){
  printf("Error: %s\n", s);
}
