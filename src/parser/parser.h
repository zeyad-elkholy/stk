#ifndef PARSER_H
#define PARSER_H
#include <stddef.h>
#include "../lexer/lexer.h"

typedef struct {
  char **args;
  int argc;

  redir *redirs;
  int redircount;
} command;

command *parse(TokenList *tokens);
void free_command(command *cmd);
#endif
