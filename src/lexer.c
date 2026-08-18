#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>


void add_token(TokenList *tokens, TokenType type, char *value)
{
    if (tokens->count >= tokens->capacity) {
        tokens->capacity *= 2;

        tokens->items = realloc(
            tokens->items,
            tokens->capacity * sizeof(Token)
        );
    }

    tokens->items[tokens->count].type = type;
    tokens->items[tokens->count].value = value;

    tokens->count++;
}


TokenList *lex(const char *input)
{
    TokenList *tokens = malloc(sizeof(TokenList));

    tokens->count = 0;
    tokens->capacity = 10;
    tokens->items = malloc(tokens->capacity * sizeof(Token));
    const char *p = input;

    while (*p) {
        // skip spaces
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        char *word = malloc(1024);
        int len = 0;

        while (*p != '\0' && *p != ' ' && *p != '\t') {

            if (*p == '\'' || *p == '"') {
                char quote = *p++;

                while (*p != '\0' && *p != quote)
                    word[len++] = *p++;

                if (*p == quote)
                    p++;
            } else {
                word[len++] = *p++;
            }
        }

        word[len] = '\0';

        add_token(tokens, TOKEN_WORD, word);
    }

    add_token(tokens, TOKEN_EOF, NULL);


    return tokens;
}

void free_tokens(TokenList *tokens)
{
    if (tokens == NULL)
        return;

    for (size_t i = 0; i < tokens->count; i++) {
        free(tokens->items[i].value);
    }

    free(tokens->items);
    free(tokens);
}
void print_tokens(TokenList *tokens)
{
    for (size_t i = 0; i < tokens->count; i++) {
        Token token = tokens->items[i];
        if (token.type == TOKEN_WORD) {
            printf("TOKEN_WORD: %s\n", token.value);
        } else if (token.type == TOKEN_EOF) {
            printf("TOKEN_EOF\n");
        }
    }
}
