%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

#include "symbol_table.h"
#include "ast.h"

extern FILE * yyin;
int yylex();
int yyerror(char *);

const int INT_TYPE = 0;
const int DOUBLE_TYPE = 1;
const int STR_TYPE = 2;
const int BOOL_TYPE = 3;
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
%type<int_val> data_type
%type<symbol> L
%type<ast> statements stmt_list stmt cond_stmt assign_stmt array_decl 
expressions expr cond_or_stmt cond_and_stmt eql_stmt comp_stmt comp_op arithmetic_stmt1 arithmetic_stmt2 unary_op_stmt variable constant loop_stmt
%%

program:            func_list  BGN statements {printf("No problem\n");}; 

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
statements:         '{' stmt_list '}' {
                               $$ = (AST*) malloc(sizeof(AST));
                               $$ = Ast_new("NA",$2,NULL); 
                        } 
                    | 
                    stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                        }
                    ;

stmt_list:          stmt_list stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,$2);
                                $$->is_stmt_list = true;
                        }
                    | stmt  {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",NULL,NULL);
                                $$->is_stmt_list = true;
                        }
                    ;

stmt:               assign_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                                $$->is_assign_stmt = true;
                        }
                    | cond_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                                $$->is_cond_stmt = true;
                        } 
                    | loop_stmt{
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                                $$->is_loop_stmt = true;
                        } 
                    | array_decl{
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                                $$->is_array_stmt = true;
                        } 
                    | expressions {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",$1,NULL);
                                $$->is_expression_stmt = true;
                        }
                    ;


assign_stmt:        data_type L SCOL {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("NA",NULL,NULL);
                                $$->is_assign_stmt = true;
                                $$->datatype = $1;
                                while(stack != NULL){
                                        Symbol* symbol = pop();
                                        symbol->type = $$->datatype;
                                        push_symbol(symbol);
                                }
                        };

L:                  L ',' ID {
                                $$ = (Symbol *) malloc(sizeof(Symbol));
                                $$ = symbol_init($1->name,-1,NULL,NULL);
                                push($$);
                        }
                    | 
                    ID      {
                                $$ = (Symbol *) malloc(sizeof(Symbol));
                                $$ = symbol_init($1,-1,NULL,NULL);
                                push($$);
                        }
                    ;

array_decl:         ARR '<' X ',' INT_CONST'>' ID SCOL{};
X:                  ARR '<' X ',' INT_CONST '>' | data_type;
                    ;


expressions:        expr SCOL {

                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                                $$->is_expressions = true;
                        }
                    ;

expr:               variable ASSIGN expr {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("=",$1,$3);
                        }
                    | variable AEQ expr {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("+=",$1,$3);
                        }
                    | variable SEQ expr {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("-=",$1,$3);
                        }
                    | variable MEQ expr {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("*=",$1,$3);
                        }
                    | variable DEQ expr {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("/=",$1,$3);
                        }
                    | variable INCR {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("++",$1,NULL);
                                $$->is_unary = true;
                        }
                    | variable DECR {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("--",$1,NULL);
                                $$->is_unary = true;
                        }
                    | cond_or_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

cond_or_stmt:       cond_or_stmt OR cond_and_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("or",$1,$3);
                        }
                    | cond_and_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

cond_and_stmt:      cond_and_stmt AND eql_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("and",$1,$3);
                        }
                    | eql_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

eql_stmt:           eql_stmt EQ comp_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("==",$1,$3);
                        }
                    | eql_stmt NEQ comp_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("!=",$1,$3);
                        }
                    | comp_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

comp_stmt:          comp_stmt comp_op arithmetic_stmt1 {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new($2->op,$1,$3);
                        }
                    | arithmetic_stmt1 {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                    }
                    ;

comp_op:            '>'     {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new(">",NULL,NULL);
                                $$->is_leaf = true;
                        }
                    | '<'  {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("<",NULL,NULL);
                                $$->is_leaf = true;
                        }
                    | GEQ  {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new(">=",NULL,NULL);
                                $$->is_leaf = true;
                        }
                    | LEQ  {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("<=",NULL,NULL);
                                $$->is_leaf = true;
                        }
                    ;

arithmetic_stmt1:   arithmetic_stmt1 ADD arithmetic_stmt2 {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("+",$1,$3);
                        }
                    | 
                    arithmetic_stmt1 SUB arithmetic_stmt2 {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("-",$1,$3);
                        }
                    | 
                    arithmetic_stmt2 {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

arithmetic_stmt2:   arithmetic_stmt2 MUL unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("*",$1,$3);
                        }
                    | arithmetic_stmt2 DIV unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("/",$1,$3);
                        }
                    | unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    ;

unary_op_stmt:      NOT unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("!",$2,NULL);
                        }
                    | ADD unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("+",$2,NULL);
                        }
                    | SUB unary_op_stmt {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("-",$2,NULL);
                        }
                    | variable {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        }
                    | constant {    
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = $1;
                        };

variable:           ID      {
                                Symbol* symbol = search_symbol($1);
                                if(symbol == NULL){
                                        printf("Identifier undeclared!!!\n");
                                        return 0;
                                }
                        }
                    | ID '(' args ')' {}
                    | arr_variable {}
                    ;

arr_variable:       ID'['expr']'
                    | arr_variable '['expr']';


cond_stmt:          IF '(' expr ')' '{'stmt_list'}' cond_stmt2 {};

cond_stmt2:         ELSE stmt | ;



loop_stmt:          LP '(' expr ')' statements {    
                                $$ = (AST*) malloc(sizeof(AST));
                                $$ = Ast_new("ifLoop",$3,$5);
                        }
                    ;


data_type:          INT {
                                $$ = INT_TYPE;
                        }
                        | 
                        DCML{
                                $$ = DOUBLE_TYPE;
                        }
                        
                        | STR {
                                $$ = STR_TYPE;
                        }
                        | BOOL {
                                $$ = BOOL_TYPE;
                        };
constant:           INT_CONST {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$->is_leaf = true;
                                $$->val.int_val = $1;
                        }
                    | DCML_CONST {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$->is_leaf = true;
                                $$->val.double_val = $1;
                        }
                    | STR_CONST {
                                $$ = (AST*) malloc(sizeof(AST));
                                $$->is_leaf = true;
                                strcpy($$->val.str_val,$1);
                        }
                    ;
/*------------------------------------------------------------*/

%%

int main(int argc, char *argv[])
{
   if (argc != 2) {
       printf("\nUsage: <exefile> <inputfile>\n");
       exit(0);
   }
   yyin = fopen(argv[1], "r");
   symbol_table = (SymbolTable*)malloc(sizeof(SymbolTable));
   symbol_table->prev = NULL;
   symbol_table->next = NULL;
   symbol_table->symbol_head = NULL;
   stack = NULL;
   yyparse();
}


int yyerror(char *s){
  printf("Error: %s\n", s);
}
