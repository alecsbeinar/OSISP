#ifndef LAB03_CHILD_H
#define LAB03_CHILD_H

#include <stddef.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include "../signals.h"

#define ALARM_COUNTS_TO_PRINT 101
#define ALARM_DELAY 10000

typedef struct {
    int O;
    int I;
} binary_t;
typedef struct {
    size_t O_O;
    size_t O_I;
    size_t I_O;
    size_t I_I;
} control_t;

void initialization(struct itimerval* timer);
void atexit_handler();
void parent_handler(int sig, siginfo_t* info, void* context);
void print_info();
void alarm_handler(int sig);
void ask_to_print();
void inform_about_print();
void reset_cycle();


#endif //LAB03_CHILD_H
