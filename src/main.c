#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
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
      if (strcmp(command + 5, "echo") == 0) {
        printf("echo is a shell builtin\n");
      } else if (strcmp(command + 5, "type") == 0) {
        printf("type is a shell builtin\n");
      } else if (strcmp(command + 5, "exit") == 0) {
        printf("exit is a shell builtin\n");
      } else if (find_in_path(command + 5) == NULL) {
        printf("%s: not found\n", command + 5);
      }else {
        printf("%s is %s\n", command + 5, find_in_path(command + 5));
      }
      continue;
    }
    printf("%s: command not found\n", command);
    
  }
  return 0;
}


char new_path[4096];

