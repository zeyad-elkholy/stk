#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_WORD,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

TokenList *lex(const char *input);
void free_tokens(TokenList *tokens);
void print_tokens(TokenList *tokens);

#endif
