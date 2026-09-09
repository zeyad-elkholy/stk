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

void builtin_type(const char *command)
{
    const char *builtins[] = {
        "echo",
        "type",
        "exit", 
        "pwd" ,
        "cd"
    };

    size_t count = sizeof(builtins) / sizeof(builtins[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(command, builtins[i]) == 0) {
            printf("%s is a shell builtin\n", command);
            return;
        }
    }

    char *path = find_in_path(command);

    if (path == NULL)
        printf("%s: not found\n", command);
    else
        printf("%s is %s\n", command, path);
}

// --------------------------------------------------
// Execute external command
// --------------------------------------------------

int execute_command(TokenList* tokens)
{
    char *args[MAX_ARGS];
    int argc = 0;
    int fd = 0;
    
      for (size_t i = 0; i < tokens->count; i++) {
        Token token = tokens->items[i];
        if (token.type != TOKEN_EOF) 
            args[argc++] = token.value;
        if(token.type == TOKEN_REDIR_OUT || token.type == TOKEN_REDIR_ERR || token.type == TOKEN_REDIR_IN) {
            if (i + 1 < tokens->count && tokens->items[i + 1].type == TOKEN_WORD) {
                char *filename = tokens->items[i + 1].value;
                if (token.type == TOKEN_REDIR_OUT) {
                    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("open");
                        return -1;
                    }
                    dup2(fd, STDOUT_FILENO);
                } else if (token.type == TOKEN_REDIR_ERR) {
                    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("open");
                        return -1;
                    }
                    dup2(fd, STDERR_FILENO);
                } else if (token.type == TOKEN_REDIR_IN) {
                    fd = open(filename, O_RDONLY);
                    if (fd < 0) {
                        perror("open");
                        return -1;
                    }
                    dup2(fd, STDIN_FILENO);
                }
                i++; // Skip the filename token
            } else {
                fprintf(stderr, "Syntax error: expected filename after redirection\n");
                return -1;
            }
        }
        
    }

    args[argc] = NULL;

    if (argc == 0)
        return 0;

    // Check whether command exists in PATH
    if (find_in_path(args[0]) == NULL)
        return -1;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);

        execvp(args[0], args);

        // execvp only returns if it failed
        perror(args[0]);
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

        char command[INPUT_SIZE];

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;
        

        command[strcspn(command, "\n")] = '\0';

        TokenList* tokens = lex(command);
        // print_tokens(tokens);
        // return 0;
        

        // Empty input
        if (command[0] == '\0')
            continue;

        // exit
        if (strcmp(command, "exit") == 0)
            break;

        // echo
        if (strncmp(command, "echo ", 5) == 0) {
          for (size_t i = 1; i < tokens->count; i++) {
            Token token = tokens->items[i];
            if (token.type == TOKEN_WORD) {
            printf("%s ", token.value);
            }
          }
          printf("\n");
            continue;
        }

        // type
        if (strncmp(command, "type ", 5) == 0) {
            builtin_type(command + 5);
            continue;
        }
        // pwd
        if (strcmp(command, "pwd") == 0) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
          printf("%s\n", cwd);
            } else {
          perror("getcwd");
            }
            continue;
        }
        // cd
        if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;
            if (strcmp(path, "~") == 0) {
                path = getenv("HOME");
            }

            if (chdir(path) != 0) {
              fprintf(stderr, "cd: %s: ", path);
              perror("");
            }
            continue;
        }

        // External command
        if (execute_command(tokens) != 0)
            printf("%s: command not found\n", command);
    }

    return 0;
}
