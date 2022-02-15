%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
extern FILE * yyin;
int yylex();
int yyerror(char *);
%}

%union {
  int int_val;
  double double_val;
}


%token ADD SUB MUL DIV ASSIGN EQ NEQ TRU FLS AND OR NOT INT DCML BOOL STR SCOL ID BEGIN
%token <int_val> INT_CONST
%token <double_val> DCML_CONS
%%

program:    func_list BEGIN statements;

/*----------------Function Declaration ----------------------*/
func_list:    func_list function |  ;
function:     data_type ID '(' params ')' statements;
params:       param_list | ;
param_list:   param_list param_type | param_type;
param_type:   data_type ID;
/*------------------------------------------------------------*/


/*------------------Statement Declaration---------------------*/
statements:   '{' stmt_list '}' | stmt;
stmt_list:    stmt_list stmt | stmt;
stmt:         assign_stmt | if_stmt | loop_stmt | array_decl | func_call;

assign_stmt:  data_type L SCOL;
L:            L, ID | ID;

if_stmt:      IF '(' condition ')' statements cond_stmt2;
cond_stmt2:   ELSE cond_stmt3 | ;
cond_stmt3:   cond_stmt | statements;

loop_stmt:    IFLOOP '(' condition ')' statements;

array_decl:   ARRAY <data_type, NUM> ID SCOL;

data_type:    INT | DCML | STR | BOOL;
/*------------------------------------------------------------*/



