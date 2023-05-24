#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[]) {
    printf("\033[1;32m"); // Green color

    puts("Child process data:");
    printf("Name: %s\n", argv[0]);
    printf("Pid: %d\n", getpid());
    printf("Ppid: %d\n", getppid());

    char buffer[256];
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("fenvp");
        exit(errno);
    }
    while (fgets(buffer, 256, file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("%s=%s\n", buffer, getenv(buffer));
    }

    printf("\033[0m"); // Turn off green color

    fclose(file);
    exit(EXIT_SUCCESS);
}
