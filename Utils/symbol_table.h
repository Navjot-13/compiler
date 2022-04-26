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
    int offset; // offset from fp
    struct Symbol* param_list;// formal parameters for when the symbol is a function
    struct Symbol* prev;
    struct Symbol* next;
} Symbol;

// Stores symbols for a particular scope
typedef struct SymbolTable {
    Symbol* symbol_head;
    int size; // size in bytes for the variables in the symbol table
    int scope;// scope number to which this symbol table belongs
    struct SymbolTable* next;
    struct SymbolTable* prev;
} SymbolTable;

extern int unused_scope; // the scope number available for usage
extern int current_scope;// the current scope number
extern SymbolTable* current_symbol_table;  // Symbol table for current scope

Symbol* symbol_init(char* name, int type, Symbol* prev, Symbol* next);

void push_symbol(Symbol* symbol);

Symbol* search_symbol(char* id);

void push_symbol_table();

void pop_symbol_table();
#endif