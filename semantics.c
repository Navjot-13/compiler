#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Utils/symbol_table.h"
#include "Utils/ast.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;

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
