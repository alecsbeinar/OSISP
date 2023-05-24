#ifndef LAB03_PARENT_H
#define LAB03_PARENT_H

#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include "../signals.h"

#define CHILD_COUNT 100
#define PAUSE_TIMER 10

typedef struct {
    pid_t pid;
    bool  print_allowed;
} child_t;

void initialization();
void atexit_handler();
void alarm_handler(int sig);
void child_set_printAllowed_all();
void set_printAllowed(child_t* child);
void kill_all_child();
pid_t kill_last_child();
void send_signal(child_t child, enum parent signal);
void child_handler(int sig, siginfo_t* info, void* context);
size_t get_child_index_by_pid(pid_t pid);

void create_child();
bool check_num(size_t num);
bool is_number_in_stdin();
size_t read_number();
void child_unset_printAllowed_all();
void unset_printAllowed(child_t* child);
void force_print(child_t child);

#endif //LAB03_PARENT_H
