#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "parent.h"

const char child_path[] = "CHILD_PATH";

int comparator(const void* str1, const void* str2){
    return strcmp(*(char**)str1, *(char**)str2);
}
void print_envp(char** envp, int len){
    qsort(envp, len, sizeof(char*), comparator);

    printf("\033[1;35m"); // Purple color
    puts("Parent environment variables:");
    for (int i = 0; i < len; ++i) {
        puts(envp[i]);
    }
    printf("\033[0m"); // Turn off purple color
}


int get_len_envp(char** envp){
    int len = 0;
    char **temp = envp;
    while(*temp)
    {
        ++temp;
        ++len;
    }
    return len;
}

char** add_variable_to_envp(char** envp){
    int len = get_len_envp(envp);
    ++len;
    char** new_envp = malloc((len + 1) * sizeof(*envp));
    for (int i = 0; i< len - 1; i++)
    {
        new_envp[i] = malloc(256 * sizeof(char*));
        strcpy(new_envp[i], envp[i]);
    }
    new_envp[len-1] = malloc(256 * sizeof(char*));
    // forming str like CHILD_PATH=...
    char buffer[256];
    strcat(strcat(strcpy(buffer, "CHILD_PATH"), "="), getenv("CHILD_PATH"));
    strcpy(new_envp[len-1], buffer);
    new_envp[len] = NULL;
    return new_envp;
}

char** create_child_env(char* file_name){
    FILE* file = fopen(file_name, "r");
    if (!file) {
        perror("fopen");
        exit(errno);
    }

    char buffer[256];
    char** env = malloc(sizeof(char*));
    int i = 0;
    while(fgets(buffer, 256, file) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0'; //read variable
        char* value_variable = getenv(buffer);
        if (value_variable) {
            env[i] = malloc((strlen(buffer) + strlen(value_variable) + 2) * sizeof(char));
            strcat(strcat(strcpy(env[i], buffer), "="), value_variable); //form str like: USER=user
            env = realloc(env, (++i + 1) * sizeof(char*));
        }
    }
    env[i] = NULL;
    return env;
}

char* get_child_path(char** env){
    while (*env) {
        if (!strncmp(*env, child_path, strlen(child_path))) {
            return *env + strlen(child_path) + 1; // skip "CHILD_PATH="
        }
        ++env;
    }
    return NULL;
}