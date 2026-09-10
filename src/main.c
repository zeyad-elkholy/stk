#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

#define MAX_ARGS 100
#define INPUT_SIZE 1048
//--------------------------------------------------
// Builtin commands
// -------------------------------------------------
const char *builtins[] = {
    "echo",
    "type",
    "exit", 
    "pwd" ,
    "cd"
};
// --------------------------------------------------
// PATH utilities
// --------------------------------------------------

char *find_in_path(const char *command)
{
    char *path = getenv("PATH");

    if (path == NULL)
        return NULL;

    char *copy = strdup(path);
    if (copy == NULL)
        return NULL;

    static char fullpath[PATH_MAX];

    char *dir = strtok(copy, ":");

    while (dir != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command);

        if (access(fullpath, X_OK) == 0) {
            free(copy);
            return fullpath;
        }

        dir = strtok(NULL, ":");
    }

    free(copy);
    return NULL;
}

// --------------------------------------------------
// Builtins
// --------------------------------------------------

int Type(const char *command,short print_path)
{

    size_t count = sizeof(builtins) / sizeof(builtins[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(command, builtins[i]) == 0) {
            if(print_path)
              printf("%s is a shell builtin\n", command);
            return 0;
        }
    }

    char *path = find_in_path(command);

    if (path == NULL){
      if(print_path)
        printf("%s: not found\n", command);
      return 1;
    }

    else{
      if(print_path)
        printf("%s is %s\n", command, path);
      return 2;
    }
}

void apply_redirection(redir *r){
  int flags;
  if (r->type == TOKEN_REDIR_IN)
  flags = O_RDONLY;
  else if (r->type == TOKEN_REDIR_OUT)
  flags = O_WRONLY | O_CREAT | O_TRUNC;

  else if (r->type == TOKEN_APPEND_OUT)
  flags = O_WRONLY | O_CREAT | O_APPEND;

  int file = open(r->file, flags, 0644);

  dup2(file, r->fd);
  close(file);
}



void execute_builtin(command* cmd, char** args)
{
    if (strcmp(cmd->args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(cmd->args[0], "type") == 0) {
        Type(args[1], 1);
    } else if (strcmp(cmd->args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
    } else if (strcmp(cmd->args[0], "cd") == 0) {
        char *path = args[1];
        if (path == NULL || strcmp(path, "~") == 0) {
            path = getenv("HOME");
        }

        if (chdir(path) != 0) {
            fprintf(stderr, "cd: %s: ", path);
            perror("");
        }
    }
}

// --------------------------------------------------
// Execute external command
// --------------------------------------------------

int execute_command(command *cmd)
{
    if(strcmp(cmd->args[0], "exit") ==0 || strcmp(cmd->args[0], "cd") ==0){
      execute_builtin(cmd, cmd->args);
      return 0;
    }

    int type = Type(cmd->args[0], 0);
    if (type == 1) {
        return -1; // Command not found
    }
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
      signal(SIGINT, SIG_DFL);
      for (int i =0; i < cmd->redircount; i++) {
        apply_redirection(&cmd->redirs[i]);
      }
      if(type == 0){
        execute_builtin(cmd, cmd->args);
        exit(EXIT_SUCCESS);
      }


      execvp(cmd->args[0], cmd->args);

      // execvp only returns if it failed
      perror(cmd->args[0]);
      exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);

    return 0;
}

// --------------------------------------------------
// Main shell
// --------------------------------------------------

int main(void)
{
    setbuf(stdout, NULL);

    while (1) {
        printf("$ ");

        char input[INPUT_SIZE];

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        

        input[strcspn(input, "\n")] = '\0';
        // Empty input
        if (input[0] == '\0')
            continue;

        TokenList* tokens = lex(input);
        command* cmd = parse(tokens);

        //
        // // exit
        // if (strcmp(input, "exit") == 0)
        //     break;
        //
        // // echo
        // if (strncmp(input, "echo ", 5) == 0) {
        //   for (size_t i = 1; i < tokens->count; i++) {
        //     Token token = tokens->items[i];
        //     if (token.type == TOKEN_WORD) {
        //     printf("%s ", token.value);
        //     }
        //   }
        //   printf("\n");
        //     continue;
        // }
        //
        // // type
        // if (strncmp(input, "type ", 5) == 0) {
        //     builtin_type(input + 5);
        //     continue;
        // }
        // // pwd
        // if (strcmp(input, "pwd") == 0) {
        //     char cwd[PATH_MAX];
        //     if (getcwd(cwd, sizeof(cwd)) != NULL) {
        //   printf("%s\n", cwd);
        //     } else {
        //   perror("getcwd");
        //     }
        //     continue;
        // }
        // // cd
        // if (strncmp(input, "cd ", 3) == 0) {
        //     char *path = input + 3;
        //     if (strcmp(path, "~") == 0) {
        //         path = getenv("HOME");
        //     }
        //
        //     if (chdir(path) != 0) {
        //       fprintf(stderr, "cd: %s: ", path);
        //       perror("");
        //     }
        //     continue;
        // }
        //
        // External input
        if (execute_command(cmd) == -1)
            printf("%s: command not found\n", input);
    }

    return 0;
}
