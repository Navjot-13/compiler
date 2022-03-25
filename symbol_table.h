#ifndef SYMBOL_H_NAME
#define SYMBOL_H_NAME

// Stores the type of one variable
typedef struct Symbol {
    char* name;
    int type;
    struct Symbol* prev;
    struct Symbol* next;
} Symbol;

Symbol* stack;

// Stores symbols for a particular scope
typedef struct SymbolTable {
    Symbol* symbol_head;
    struct SymbolTable* next;
    struct SymbolTable* prev;
} SymbolTable;

SymbolTable* symbol_table;  // Symbol table for current scope

Symbol* symbol_init(char* name, int type, Symbol* prev, Symbol* next) {
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = malloc(strlen(name) * sizeof(char));
    new_symbol->type = type;
    new_symbol->prev = prev;
    new_symbol->next = next;
    return new_symbol;
}

void push_symbol(Symbol* symbol) {
    if (symbol_table->symbol_head == NULL) {
        symbol_table->symbol_head = symbol_init(symbol->name, symbol->type, NULL, NULL);
    }
    Symbol* cur_symbol = symbol_table->symbol_head;
    while (cur_symbol->next != NULL) {
        cur_symbol = cur_symbol->next;
    }
}

Symbol* search_symbol(char* id) {
    Symbol* required = NULL;
    SymbolTable* cur_symbol_table = symbol_table;
    while (cur_symbol_table != NULL) {
        Symbol* cur_symbol = cur_symbol_table->symbol_head;
        while (cur_symbol != NULL) {
            if (strcmp(cur_symbol->name, id) == 0) {
                return cur_symbol;
            }
            cur_symbol = cur_symbol->next;
        }
        cur_symbol_table = cur_symbol_table->prev;
    }
    return NULL;
}

void push(Symbol* symbol) {
    if (stack == NULL) {
        stack = symbol_init(symbol->name, symbol->type, NULL, NULL);
    }
    Symbol* cur_symbol = stack;
    while (cur_symbol->next != NULL) {
        cur_symbol = cur_symbol->next;
    }
    cur_symbol->next = symbol_init(symbol->name, symbol->type, cur_symbol, NULL);
}

Symbol* pop() {
    if (stack == NULL) {
        printf("Stack is empty!!");
        return NULL;
    }
    Symbol* cur_symbol = stack;
    while (cur_symbol->next != NULL) {
        cur_symbol = cur_symbol->next;
    }
    if (cur_symbol->prev) {
        cur_symbol->prev->next = NULL;
    } else {
        stack = NULL;
    }
    return cur_symbol;
}

#endif