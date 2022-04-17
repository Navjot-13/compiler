#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"


void pop_symbol_table() {
    SymbolTable* latest_symbol_table = current_symbol_table->prev;
    free(current_symbol_table);
    current_symbol_table = latest_symbol_table;
    if(current_symbol_table != NULL){
        current_symbol_table->next = NULL;
    }
}

void push_symbol_table() {
    if(current_symbol_table == NULL){
        current_symbol_table = (SymbolTable*)malloc(sizeof(SymbolTable));
        current_symbol_table->prev = NULL;
        current_symbol_table->next = NULL;
        current_symbol_table->symbol_head = NULL;
    } else {
        current_symbol_table->next = (SymbolTable*) malloc(sizeof(SymbolTable));
        current_symbol_table->next->prev = current_symbol_table;
        current_symbol_table = current_symbol_table->next;
        current_symbol_table->next = NULL;
        current_symbol_table->symbol_head = NULL;
    }
    current_symbol_table->scope = current_scope;
}

void push_persistent_symbol_table(){
    if(persistent_symbol_table == NULL){
        persistent_symbol_table = (SymbolTable*)malloc(sizeof(SymbolTable));
        persistent_symbol_table->prev = NULL;
        persistent_symbol_table->next = NULL;
        persistent_symbol_table->symbol_head = NULL;
    } else {
        persistent_symbol_table->next = (SymbolTable*) malloc(sizeof(SymbolTable));
        persistent_symbol_table->next->prev = persistent_symbol_table;
        persistent_symbol_table = persistent_symbol_table->next;
        persistent_symbol_table->next = NULL;
        persistent_symbol_table->symbol_head = NULL;
    }
    persistent_symbol_table->scope = current_scope;
}

Symbol* search_symbol(char* id) {
    // printf("REACHED for %s\n",id);
    SymbolTable* cur_symbol_table = current_symbol_table;
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
    // printf("Search: %s\n",id);
    return NULL;
}

void push_symbol(Symbol* symbol) {
    char *name = (char*)malloc((strlen(symbol->name)+1)*sizeof(char));
    strcpy(name, symbol->name);
    Symbol* new_symbol = symbol_init(name,symbol->type,NULL,NULL);
    new_symbol->is_array = symbol->is_array;
    new_symbol->is_function = symbol->is_function;
    new_symbol->param_list = symbol->param_list;
    new_symbol->size = symbol->size;
    new_symbol->offset = symbol->offset;
    if (current_symbol_table->symbol_head == NULL) {
        current_symbol_table->symbol_head = new_symbol;
    } else {
        Symbol* cur_symbol = current_symbol_table->symbol_head;
        while (cur_symbol->next != NULL) {
            cur_symbol = cur_symbol->next;
        }
        cur_symbol->next = new_symbol;
        new_symbol->prev = cur_symbol;
    }
    push_persistent_symbol(symbol);
}

void adjust_persistent_symbol_table() {
    SymbolTable* cur_symbol_table = persistent_symbol_table;
    while(cur_symbol_table->next != NULL){
        cur_symbol_table = cur_symbol_table->next;
    }
    while(cur_symbol_table->scope != current_scope){
        cur_symbol_table = cur_symbol_table->prev;
    }
    persistent_symbol_table = cur_symbol_table;
}

void push_persistent_symbol(Symbol* symbol){
    char *name = (char*)malloc((strlen(symbol->name)+1)*sizeof(char));
    strcpy(name, symbol->name);
    Symbol* new_symbol = symbol_init(name,symbol->type,NULL,NULL);
    new_symbol->is_array = symbol->is_array;
    new_symbol->is_function = symbol->is_function;
    new_symbol->param_list = symbol->param_list;
    new_symbol->size = symbol->size;
    if (persistent_symbol_table->symbol_head == NULL) {
        persistent_symbol_table->symbol_head = new_symbol;
        persistent_symbol_table->size = symbol->size;
    } else {
        Symbol* cur_symbol = persistent_symbol_table->symbol_head;
        while (cur_symbol->next != NULL) {
            cur_symbol = cur_symbol->next;
        }
        persistent_symbol_table->size += symbol->size;
        cur_symbol->next = new_symbol;
        new_symbol->prev = cur_symbol;
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
    new_symbol->size = 0;
    new_symbol->offset=0;
    return new_symbol;
}