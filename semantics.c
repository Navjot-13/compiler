#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Utils/ast.h"
#include "Utils/symbol_table.h"
#include "y.tab.h"

extern FILE *yyin;
extern AST *astroot;

void traverse(AST *astroot);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("\nUsage: <exefile> <inputfile>\n");
        exit(0);
    }
    yyin = fopen(argv[1], "r");
    symbol_table = (SymbolTable *)malloc(sizeof(SymbolTable));
    symbol_table->prev = NULL;
    symbol_table->next = NULL;
    symbol_table->symbol_head = NULL;
    stack = NULL;
    yyparse();
    printf("Hello\n");
    traverse(astroot);
}

void traverse(AST *astroot) {
    if (astroot == NULL)
        return;
    traverse(astroot->left);
    printf("%d\n", astroot->type);
    traverse(astroot->right);
}