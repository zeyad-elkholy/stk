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
    int in_single = 0;
    int in_double = 0;

    while (*p != '\0') {

        // Outside quotes: whitespace ends the word
        if (!in_single && !in_double &&
            (*p == ' ' || *p == '\t')) {
            break;
        }

        // Backslash
        if (*p == '\\') {
            if (in_single) {
                // In single quotes, backslash is literal
                word[len++] = *p++;
            }
            else if (in_double) {
                // In double quotes, only escape these specially
                if (p[1] == '"' || p[1] == '\\' || p[1] == '$') {
                    p++;
                    word[len++] = *p++;
                }
                else {
                    // Backslash stays literal
                    word[len++] = *p++;
                }
            }
            else {
                // Outside quotes, escape the next character
                p++;

                if (*p != '\0')
                    word[len++] = *p++;
            }

            continue;
        }
        if(*p == '>' && !in_single && !in_double) {
            if (len > 0) {
                word[len] = '\0';
                add_token(tokens, TOKEN_WORD, word);
                word = malloc(1024);
                len = 0;
            }

            if (*(p + 1) == '>') {
                add_token(tokens, TOKEN_APPEND, NULL);
                p += 2;
            } else {
                add_token(tokens, TOKEN_REDIR_OUT, NULL);
                p++;
            }
            continue;
        }
        if((*p == '<'  || *p == '2')&& !in_single && !in_double) {
            if (len < 0) {
                word[len] = '\0';
                add_token(tokens, TOKEN_WORD, word);
                word = malloc(1024);
                len = 0;
            }
            if (*p == '2') {
                if (*(p + 1) == '>') {
                    add_token(tokens, TOKEN_REDIR_ERR, NULL);
                    p += 2;
                } else {
                    word[len++] = *p++;
                }
                continue;
            } 

            if (*(p + 1) == '<') {
                add_token(tokens, TOKEN_APPEND, NULL);
                p += 2;
            } else {
                add_token(tokens, TOKEN_REDIR_OUT, NULL);
                p++;
            }
            continue;
        }

        // Single quote
        if (*p == '\'' && !in_double) {
            in_single = !in_single;
            p++;
            continue;
        }

        // Double quote
        if (*p == '"' && !in_single) {
            in_double = !in_double;
            p++;
            continue;
        }

        // Normal character
        word[len++] = *p++;
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
