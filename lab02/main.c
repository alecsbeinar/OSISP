#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "parent/parent.h"

extern char** environ;
extern const char child_path[];

int main(int argc, char* argv[], char* envp[]) {
    if (argc != 2) {
        fprintf(stderr, "Add 1 argument: env file path\n");
        exit(EXIT_FAILURE);
    }

    //add variable CHILD_PATH to environ
    char filename[] = "./chld";
    char var[256] = "CHILD_PATH=";
    strcat(var, realpath(filename, NULL));

    if (putenv(var) == -1)
    {   printf("putenv failed: out of memory");
        exit(1);
    }

    //add variable CHILD_PATH to envp
    envp = add_variable_to_envp(envp);

    print_envp(envp, get_len_envp(envp));

    //create child envp specified in each computer
    char** child_env = create_child_env(argv[1]); // argv[1] is env file path

    int choice;
    size_t child_count = 0;
    while(true)
    {
        printf("[+] - child process with getenv()\n"
               "[*] - child process with envp[]\n"
               "[&] - child process with environ\n"
               "[q] - exit\n"
               ">");
        choice = getchar();
        getchar();

        if (choice == 'q')
            exit(EXIT_SUCCESS);
        else if (choice != '+' && choice != '*' && choice != '&')
            exit(EXIT_FAILURE);

        // Get path to file child for execute
        char* child_exec = NULL;
        switch ((char) choice) {
            case '+':
                child_exec = getenv(child_path);
                break;

            case '*':
                child_exec = get_child_path(envp);
                break;

            case '&':
                child_exec = get_child_path(environ);
                break;
        }

        // Create new child name
        char child_name[10];
        sprintf(child_name, "child_%zu", child_count++);

        // args for pass to child
        char* const args[] = {child_name, argv[1], NULL};

        pid_t pid = fork();
        if (pid > 0) {
            // Parent process
            int status;
            wait(&status);
        } else if (pid == 0) {
            // Child process
            if (execve(child_exec, args, child_env) == -1) {
                perror("execve");
                exit(errno);
            }
        } else {
            perror("fork");
            exit(errno);
        }
    }
}

