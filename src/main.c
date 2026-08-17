#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
// find path helper for type builtin command
char *find_in_path(const char *command){
    char *path = getenv("PATH");
    if (!path)
        return NULL;

    char *copy = strdup(path);
    if (!copy)
        return NULL;

    static char fullpath[PATH_MAX];

    char *dir = strtok(copy, ":");

    while (dir) {
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
// type builtin command
void type(const char *command){
  char* BuiltinCommands[] ={"echo", "type", "exit"};
  int size = sizeof(BuiltinCommands);
 for (int i = 0 ;i < size;i++) {
      if (strcmp(command, BuiltinCommands[i]) ==0) {
        printf("%s is a shell builtin\n", command);
        return;
      }
 } 
   if (find_in_path(command) == NULL) {
    printf("%s: not found\n", command );
  }else {
    printf("%s is %s\n", command , find_in_path(command));
  }
}
void ExecuteCommand(char *command) {
    char *args[100];

    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL && i < 99) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    
        pid_t pid = fork();

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            execvp(args[0], args);
            printf("%s: command not found\n", args[0]);
        } 
        else if (pid > 0) {
            waitpid(pid, NULL, 0); 
            
            if (strcmp(args[0], "cat") == 0) {
                printf("\n");
            }
        } 

}
int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);
  while (1) {
    // TODO: Uncomment the code below to pass the first stage
    printf("$ ");
    char command[1048];
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = 0; // Remove newline character
    if (strcmp(command, "exit") == 0) {
      break;
    }else if (strncmp(command, "echo ",5) == 0){
      printf("%s\n", command + 5);
      continue;
    }else if (strncmp(command, "type ",5) == 0){
      type(command + 5);
      continue;
    }else{
      if (find_in_path(command) != NULL) {
        ExecuteCommand(command);
        continue;}
      else{;}
}
    printf("%s: command not found\n", command);
    
  }
  return 0;
}




