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
func_list:  func_list function |  ;
function:   data_type ID '(' params ')' statements;
params:     param_list | ;
param_list: param_list param_type | param_type;
param_type: data_type ID;
/*------------------------------------------------------------*/


statements: '{' stmt_list '}' | stmt;
stmt_list:  stmt_list stmt | stmt;
stmt:       


decl_stmts: decl_stmts decl | decl;

decl:       var_decl | fun_decl;


var_decl:   datatype L SCOL;

L:          L, ID | ID;


