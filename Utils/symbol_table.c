#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"


void pop_symbol_table() {
    SymbolTable* latest_symbol_table = symbol_table->prev;
    free(symbol_table);
    symbol_table = latest_symbol_table;
    symbol_table->next = NULL;
}

void push_symbol_table() {
    if(symbol_table == NULL){
        symbol_table = (SymbolTable*)malloc(sizeof(SymbolTable));
        symbol_table->prev = NULL;
        symbol_table->next = NULL;
        symbol_table->symbol_head = NULL;
        global_symbol_table = symbol_table;
    } else {
        symbol_table->next = (SymbolTable*) malloc(sizeof(SymbolTable));
        symbol_table->next->prev = symbol_table;
        symbol_table = symbol_table->next;
        symbol_table->next = NULL;
        symbol_table->symbol_head = NULL;
    }
}

Symbol* search_symbol(char* id) {
    // printf("Search: %s\n",id);
    SymbolTable* cur_symbol_table = symbol_table;
    while (cur_symbol_table != NULL) {
        Symbol* cur_symbol = cur_symbol_table->symbol_head;
        while (cur_symbol != NULL) {
            // printf("Performing comparison in symbol table: %s,%s,%d\n",cur_symbol->name,id,strcmp(cur_symbol->name,id));
            if (strcmp(cur_symbol->name, id) == 0) {
                // printf("Match found: %s\n",cur_symbol->name);
                return cur_symbol;
            }
            cur_symbol = cur_symbol->next;
        }
        cur_symbol_table = cur_symbol_table->prev;
    }
    return NULL;
}

Symbol* global_search_symbol(char* id) {
    printf("Search: %s\n",id);
    SymbolTable* cur_symbol_table = global_symbol_table;
    Symbol* cur_symbol = cur_symbol_table->symbol_head;
    while (cur_symbol != NULL) {
        printf("Performing comparison in symbol table: %s,%s,%d\n",cur_symbol->name,id,strcmp(cur_symbol->name,id));
        if (strcmp(cur_symbol->name, id) == 0) {
            printf("Match found: %s\n",cur_symbol->name);
            return cur_symbol;
        }
        cur_symbol = cur_symbol->next;
    }
    cur_symbol_table = cur_symbol_table->prev;
    return NULL;
}

void push_symbol(Symbol* symbol) {
    if (symbol_table->symbol_head == NULL) {
        // symbol_table->symbol_head = symbol_init(symbol->name, symbol->type, NULL, NULL);
        symbol_table->symbol_head = symbol;
    } else {
        // I changed function check once
        Symbol* cur_symbol = symbol_table->symbol_head;
        while (cur_symbol->next != NULL) {
            cur_symbol = cur_symbol->next;
        }
        // cur_symbol->next = symbol_init(symbol->name, symbol->type, cur_symbol, NULL);
        cur_symbol->next = symbol;
    }
}

void push_global_symbol(Symbol* symbol) {
    if (global_symbol_table->symbol_head == NULL) {
        // symbol_table->symbol_head = symbol_init(symbol->name, symbol->type, NULL, NULL);
        global_symbol_table->symbol_head = symbol;
    } else {
        // I changed function check once
        Symbol* cur_symbol = global_symbol_table->symbol_head;
        while (cur_symbol->next != NULL) {
            cur_symbol = cur_symbol->next;
        }
        // cur_symbol->next = symbol_init(symbol->name, symbol->type, cur_symbol, NULL);
        cur_symbol->next = symbol;
    }
}

Symbol* symbol_init(char* name, int type, Symbol* prev, Symbol* next) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = (char*)malloc((strlen(name)+1) * sizeof(char));
    new_symbol->name = name;
    new_symbol->type = type;
    new_symbol->prev = prev;
    new_symbol->next = next;
    new_symbol->is_function = -1;
    new_symbol->is_array = -1;
    new_symbol->param_list = NULL;
    return new_symbol;
}