#ifndef SYMBOL_H_NAME
#define SYMBOL_H_NAME

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define INT_TYPE 0
#define DOUBLE_TYPE 1
#define STR_TYPE 2
#define BOOL_TYPE 3
#define ARRAY_TYPE 4

// Stores the information of one variable
typedef struct Symbol {
    char* name;
    int type;
    int is_function;
    int is_array;
    int size;
    struct Symbol* prev;
    struct Symbol* next;
} Symbol;

extern Symbol* stack;

// Stores symbols for a particular scope
typedef struct SymbolTable {
    Symbol* symbol_head;
    struct SymbolTable* next;
    struct SymbolTable* prev;
} SymbolTable;

extern SymbolTable* symbol_table;  // Symbol table for current scope

Symbol* symbol_init(char* name, int type, Symbol* prev, Symbol* next);

void push_symbol(Symbol* symbol);

Symbol* search_symbol(char* id);

void push_symbol_table();

void pop_symbol_table();

// Stack functions
void push(Symbol* symbol);

Symbol* pop();
#endif