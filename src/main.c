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
    "cd"  ,
    "jobs"
};
// --------------------------------------------------
// Steve's job table implementation
// --------------------------------------------------
#define MAX_JOBS 64

typedef struct {
    int id;             // 1-based job number ([1], [2], etc.)
    pid_t pid;          // Process ID
    char command[256];  // Command string for display
    int active;         // 1 if active, 0 if free for recycling
} Job;

Job job_table[MAX_JOBS];
// Helper to find the highest active job ID for the '+' indicator
int get_max_job_id(void) {
    int max_id = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].active && job_table[i].id > max_id) {
            max_id = job_table[i].id;
        }
    }
    return max_id;
}

// Always scans from index 0 to assign the lowest available Job ID (Recycling)
int add_job(pid_t pid, char **args) {
    char cmd_str[256] = "";
    for (int i = 0; args[i] != NULL; i++) {
        if (i > 0) strncat(cmd_str, " ", sizeof(cmd_str) - strlen(cmd_str) - 1);
        strncat(cmd_str, args[i], sizeof(cmd_str) - strlen(cmd_str) - 1);
    }

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!job_table[i].active) {
            job_table[i].id = i + 1;
            job_table[i].pid = pid;
            strncpy(job_table[i].command, cmd_str, sizeof(job_table[i].command) - 1);
            job_table[i].active = 1;

            printf("[%d] %d\n", job_table[i].id, pid);
            return job_table[i].id;
        }
    }
    fprintf(stderr, "shell: job table full\n");
    return -1;
}

// Non-blocking reap executed immediately before printing the prompt
void reap_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int max_id = get_max_job_id();

        for (int i = 0; i < MAX_JOBS; i++) {
            if (job_table[i].active && job_table[i].pid == pid) {
                // Print exact CodeCrafters formatted output
                if (job_table[i].id == max_id) {
                    printf("[%d]+  %-24s%s\n", job_table[i].id, "Done", job_table[i].command);
                } else {
                    printf("[%d]   %-24s%s\n", job_table[i].id, "Done", job_table[i].command);
                }

                // Free slot so ID is recycled for the next background process
                job_table[i].active = 0;
                job_table[i].pid = 0;
                break;
            }
        }
    }
}

// Print running jobs for `jobs` builtin
void print_jobs(void) {
    int max_id = get_max_job_id();

    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].active) {
            if (job_table[i].id == max_id) {
                printf("[%d]+  %-24s%s\n", job_table[i].id, "Running", job_table[i].command);
            } else {
                printf("[%d]   %-24s%s\n", job_table[i].id, "Running", job_table[i].command);
            }
        }
    }
}
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
    } else if (strcmp(cmd->args[0], "jobs") == 0) {
        print_jobs();
    }
}

// --------------------------------------------------
// Execute external command
// --------------------------------------------------

int execute_command(command *cmd)
{
    if(strcmp(cmd->args[0], "exit") ==0 || strcmp(cmd->args[0], "cd") ==0 || strcmp(cmd->args[0], "jobs") ==0){
      execute_builtin(cmd, cmd->args);
      return 0;
    }

    int type = Type(cmd->args[0], 0);
    if (type == 1) {
        return -1; // Command not found
    }
    // printf("background = %d\n", cmd->is_background);
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
    if (!cmd->is_background){
    waitpid(pid, NULL, 0);}
    else {
      add_job(pid, cmd->args);
    
    }


    return 0;
}

// --------------------------------------------------
// Main shell
// --------------------------------------------------

int main(void)
{
    setbuf(stdout, NULL);

    while (1) {
        reap_jobs();
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

        if (execute_command(cmd) == -1)
            printf("%s: command not found\n", input);
    }

    return 0;
}
