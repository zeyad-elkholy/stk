#include <ctype.h>
#include <stdlib.h>
#include "parser.h"
command *parse(TokenList *tokens){
  command *cmd = malloc(sizeof(command));
  cmd->args = malloc(sizeof(char) * (tokens->count+ 1));
  cmd->redirs = malloc(sizeof(redir) * (tokens->count+ 1));
  cmd->argc = 0;
  cmd->redircount= 0;


  for(int i = 0; i < tokens->count; i++){
    Token *token = &tokens->items[i];
    if(token->type == TOKEN_WORD){
      cmd->args[cmd->argc++] = token->value;
    }else if(token->type == TOKEN_APPEND_OUT || token->type == TOKEN_APPEND_IN|| token->type == TOKEN_REDIR_IN || token->type == TOKEN_REDIR_OUT){
      Token *filename = &tokens->items[++i];

      if(isdigit((unsigned char)token->value[0])){
        cmd->redirs[cmd->redircount].fd = atoi(token->value);
        cmd->redirs[cmd->redircount].type = token->type;
        cmd->redirs[cmd->redircount++].file = filename->value;
      }else {
        switch (token->type) {
          case TOKEN_REDIR_IN:
          case TOKEN_APPEND_IN:
            cmd->redirs[cmd->redircount].fd = 0;
            break;
          case TOKEN_APPEND_OUT:
          case TOKEN_REDIR_OUT:
            cmd->redirs[cmd->redircount].fd = 1;
            break;
        }
        cmd->redirs[cmd->redircount].type = token->type;
        cmd->redirs[cmd->redircount++].file = filename->value;
      }

    }
  }
  cmd->args[cmd->argc] = NULL;
  return cmd;
}
void free_command(command *cmd){
  if(cmd == NULL) return;
  for(int i = 0; i < cmd->argc; i++){
    free(cmd->args[i]);
  }
  free(cmd->args);
  free(cmd->redirs);
  free(cmd);
}

