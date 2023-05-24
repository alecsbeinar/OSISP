#include "child.h"

extern binary_t data;
extern control_t control;
extern int alarm_count;
extern bool output_allowed;

// -------------------- Init Functions --------------------

void initialization(struct itimerval* timer) {
    // Init atexit
    if (atexit(atexit_handler)) {
        fputs("atexit failed", stderr);
        exit(EXIT_FAILURE);
    }

    // Init parent signals handler
    struct sigaction parent_signals;
    parent_signals.sa_flags     = SA_SIGINFO; //Invoke signal-catching function with three arguments instead of one.
    parent_signals.sa_sigaction = parent_handler;

    if (sigaction(SIGUSR1, &parent_signals, NULL)) {
        perror("sigaction");
        exit(errno);
    }

    // Init alarm handler
    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("signal");
        exit(errno);
    }

    // Init timer - call alarm() function after period of time
    timer->it_value.tv_sec     = 0;             // how long will it notify
    timer->it_value.tv_usec    = ALARM_DELAY;
    timer->it_interval.tv_sec  = 0;             // after what period of time will notify
    timer->it_interval.tv_usec = ALARM_DELAY;

    if (setitimer(ITIMER_REAL, timer, NULL)) {  // ITIMER_REAL calculate by total time, not for program
        perror("setitimer");
        exit(errno);
    }
}

void atexit_handler() {
    printf("Process with pid=%d is ended\n", getpid());
}

void parent_handler(int sig, siginfo_t* info, void* context) {
    switch (info->si_int) {
        case PARENT_RESPONSE:
            output_allowed = true;
            break;

        case PARENT_FORCE_PRINT:
            print_info();
            break;

        case PARENT_KILL:
            exit(EXIT_SUCCESS);

        default:
            fprintf(stderr,
                    "Unknown signal occurred in parent_handler: %d\n",
                    info->si_int);
            exit(EXIT_FAILURE);
    }
}

void print_info() {
    // Output is carried out character by character
    char buffer[1024];
    char* ptr = buffer;
    sprintf(buffer, "ppid=%d, pid=%d, stat={%zu, %zu, %zu, %zu}\n",
            getppid(), getpid(), control.O_O, control.O_I, control.I_O, control.I_I);
    while (*ptr) {
        putchar(*ptr);
        ++ptr;
    }
}

void alarm_handler(int sig) {
    data.I == 0 ? (data.O == 0 ? ++control.O_O
                               : ++control.O_I)
                : (data.O == 0) ? ++control.I_O
                                : ++control.I_I;

    ++alarm_count;
}

// -------------------- Init Functions --------------------

void ask_to_print() {
    union sigval value = {CHILD_ASK};
    if (sigqueue(getppid(), SIGUSR2, value)) {
        perror("sigqueue");
        exit(errno);
    }
}

void inform_about_print() {
    union sigval value = {CHILD_INFORM};
    if (sigqueue(getppid(), SIGUSR2, value)) {
        perror("sigqueue");
        exit(errno);
    }
}

void reset_cycle() {
    alarm_count    = 0;
    output_allowed = false;
    control.O_O = 0;
    control.O_I = 0;
    control.I_O = 0;
    control.I_I = 0;
}

