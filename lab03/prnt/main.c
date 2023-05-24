#include "parent.h"

size_t count = 0;
child_t child_array[CHILD_COUNT];

int main() {
    initialization();

    printf("[+] - create child process\n"
           "[-] - kill last child process\n"
           "[k] - kill all add child processes\n"
           "[s]<num> - stop statistic\n"
           "[g]<num> - resume statistic\n"
           "[p]num - print C_{num} statistic and freeze other childs\n"
           "[q] - exit\n"
           ">");
    while (1) {
        switch (getchar()) {
            case '+':
                create_child();
                printf("Child with pid=%d was created\n", child_array[count - 1].pid);
                break;

            case '-':
                kill_last_child();
                printf("Last child was killed, %zu left\n", count);
                break;

            case 'k':
                kill_all_child();
                puts("All child process was killed");
                break;

            case 's':
                if (is_number_in_stdin()) {
                    size_t num = read_number();
                    if (check_num(num)) {
                        unset_printAllowed(&child_array[num]);
                    }
                } else {
                    child_unset_printAllowed_all();
                }
                break;

            case 'g':
                if (is_number_in_stdin()) {
                    size_t num = read_number();
                    if (check_num(num)) {
                        set_printAllowed(&child_array[num]);
                    }
                } else {
                    child_set_printAllowed_all();
                }
                break;

            case 'p':
                if (!is_number_in_stdin()) {
                    fputs("Invalid command! Correct form is p<num>", stderr);
                    exit(EXIT_FAILURE);
                }
                size_t num = read_number();

                child_unset_printAllowed_all();
                force_print(child_array[num]);
                alarm(PAUSE_TIMER);
                break;

            case 'q':
                exit(EXIT_SUCCESS);

            default:
                break;
        }
    }
}
