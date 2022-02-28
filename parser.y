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
  char str_val[300];
}


%token ADD SUB MUL DIV ASSIGN EQ NEQ TRU FLS AND OR NOT INT DCML BOOL STR SCOL ID BEGIN AEQ MEQ SEQ DEQ INCR DECR
%token <int_val> INT_CONST
%token <double_val> DCML_CONST
%token <str_val> STR_CONST
%%

program:            func_list BEGIN statements;

/*----------------Function Declaration ----------------------*/
func_list:          func_list function |  ;

function:           data_type ID '(' params ')' statements;

params:             param_list 
                    | ;

param_list:         param_list param_type 
                    | param_type;

param_type:         data_type ID;

args:               arg_list
                    | ;

arg_list:           arg_list',' expr
                    | expr;        
/*------------------------------------------------------------*/



/*------------------Statement Declaration---------------------*/
statements:         '{' stmt_list '}' | stmt;

stmt_list:          stmt_list stmt | stmt;

stmt:               assign_stmt | if_stmt | loop_stmt | array_decl | expressions;


assign_stmt:        data_type L SCOL;

L:                  L',' ID | ID;

array_decl:         ARRAY '<'data_type',' NUM'>' ID SCOL;


expressions:        expr SCOL;

expr:               variable ASSIGN expr
                    | variable AEQ expr
                    | variable SEQ expr
                    | variable MEQ expr
                    | variable DEQ expr
                    | variable INCR
                    | variable DECR
                    | cond_or_stmt;

variable:           ID
                    | ID '(' args ')'
                    | arr_variable;

arr_variable:       ID'['expr']'
                    | arr_variable '['expr ']';

cond_or_stmt:       cond_or_stmt OR cond_and_stmt
                    | cond_and_stmt;

cond_and_stmt:      cond_and_stmt AND eql_stmt
                    | eql_stmt;

eql_stmt:           eql_stmt EQ comp_stmt
                    | eql_stmt NEQ comp_stmt
                    | comp_stmt;

comp_stmt:          comp_stmt comp_op arithmetic_stmt1
                    | arithmetic_stmt1;

comp_op:            '>'
                    | '<'
                    | '>='
                    | '<=';

arithmetic_stmt1:   arithmetic_stmt1 ADD arithmetic_stmt2
                    | arithmetic_stmt1 SUB arithmetic_stmt2
                    | arithmetic_stmt2;

arithmetic_stmt2:   arithmetic_stmt2 MUL unary_op_stmt
                    | arithmetic_stmt2 DIV unary_op_stmt
                    | unary_op_stmt;

unary_op_stmt:      NOT unary_op_stmt
                    | ADD unary_op_stmt
                    | SUB unary_op_stmt
                    | variable
                    | constant;


cond_stmt:          IF '(' expr ')' statements cond_stmt2;

cond_stmt2:         ELSE cond_stmt3 | ;

cond_stmt3:         cond_stmt | statements;


loop_stmt:          IFLOOP '(' expr ')' statements;


data_type:          INT | DCML | STR | BOOL;
constant:           INT_CONST | DCML_CONST | STR_CONST;
/*------------------------------------------------------------*/

%%

int main(int argc, char *argv[])
{
   if (argc != 2) {
       printf("\nUsage: <exefile> <inputfile>\n");
       exit(0);
   }
   yyin = fopen(argv[1], "r");
  yyparse();
}


int yyerror(char *s){
  printf("\n\nError: %s\n", s);
}

