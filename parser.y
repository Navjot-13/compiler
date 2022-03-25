%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
extern FILE * yyin;
int yylex();
int yyerror(char *);

const int INT_TYPE = 0;
const int DOUBLE_TYPE = 1;
const int STR_TYPE = 2;
const int BOOL_TYPE = 3;

typedef struct Symbol{
        char *name;
        int type;
} Symbol;

typedef struct SymbolTable{
        Symbol symbolNode;  
        struct SymbolTable *next;
} SymbolTable;
SymbolTable *symbolTable;

typedef struct AST{
        char *op;
        bool is_unary;
        bool is_leaf;
        bool is_array;
        bool is_expressions;
        bool is_stmt_list;
        bool is_assign_stmt;
        bool is_cond_stmt;
        bool is_loop_stmt;
        bool is_array_stmt;
        bool is_condition_stmt;
        bool is_expression_stmt;
        union {
                int int_val;
                double double_val;
                char str_val[300];
        } val;
        struct AST *left;
        struct AST *right;

} AST;

AST* Ast_new(char *op,AST* left,AST* right){
        AST* ast = malloc(sizeof(AST));
        strcpy(ast->op,op);
        ast->left = left;
        ast->right = right;
        ast->is_unary = false;
        ast->is_leaf = false;
        ast->is_expressions = false;
        ast->is_stmt_list = false;
        ast->is_assign_stmt = false;
        ast->is_cond_stmt = false;
        ast->is_loop_stmt = false;
        ast->is_array_stmt = false;
        ast->is_condition_stmt = false;
        return ast;
}

void addType (Symbol new_symbol){
        if(symbolTable == NULL){
                symbolTable = (SymbolTable*) malloc(sizeof(SymbolTable));
                symbolTable->next = NULL;
                symbolTable->symbolNode = new_symbol;
        }
        SymbolTable* cur_node = symbolTable;
        while(cur_node->next != NULL){
                cur_node = cur_node->next;
        }
        cur_node->next = (SymbolTable*) malloc(sizeof(SymbolTable));
        cur_node->next->next = NULL;
        cur_node->next->symbolNode = new_symbol;
}

%}

%union {
  int int_val;
  double double_val;
  char str_val[300];
  struct AST *ast;
}


%token ADD SUB MUL DIV ASSIGN EQ NEQ TRU FLS AND OR NOT INT DCML BOOL STR SCOL ID BGN AEQ MEQ SEQ DEQ INCR DECR GEQ LEQ ARR IF ELSE LP BRK RETURN CMNT MLTI_CMNT INP
%token <int_val> INT_CONST 
%token <double_val> DCML_CONST
%token <str_val> STR_CONST

%type<int_val> data_type
%type<ast> statements stmt_list stmt assign_stmt cond_stmt array_decl L
expressions expr cond_or_stmt cond_and_stmt eql_stmt comp_stmt comp_op arithmetic_stmt1 arithmetic_stmt2 unary_op_stmt variable constant loop_stmt
%%

program:            func_list  BGN {printf("re\n");}statements {printf("No problem\n");}; 

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


assign_stmt:        data_type {printf("hello\n");} L SCOL {
                                $$ = (AST*) malloc(sizeof(AST));
                                printf("reached\n");
                        };

L:                  L ',' ID 
                    | 
                    ID {
                                $$ = (AST*) malloc(sizeof(AST));
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

variable:           ID {}
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
                                printf("hellooo\n");
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
   symbolTable = NULL;
   yyparse();
}


int yyerror(char *s){
  printf("Error: %s\n", s);
}
