#include "parent.h"

extern size_t count;
extern child_t child_array[CHILD_COUNT];

// -------------------- Init Functions --------------------

void initialization(){
    // Init atexit
    // atexit_handler kill all child-processes; send data PARENT_KILL to last child by signal SIGUSR1
    if (atexit(atexit_handler)) {
        fputs("atexit failed", stderr);
        exit(EXIT_FAILURE);
    }

    // Init alarm handler
    // SIGALRM - signal that send process by alarm() in specified time (default call alarm(0) - never send signal)
    // alarm_handler set print_allowed = true in all child
    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("signal");
        exit(errno);
    }

    // Init child signals handler
    // By specified user signal SIGUSR2 catch signals from child in func child_handler
    struct sigaction child_signals;
    child_signals.sa_flags     = SA_SIGINFO;      // 3 arguments in func; set func
    child_signals.sa_sigaction = child_handler;   // child_handler will be also execute when parent receive SIGUSR2

    if (sigaction(SIGUSR2, &child_signals, NULL)) {  // old_act - NULL: not saved last func-handler
        perror("sigaction");
        exit(errno);
    }
}

void atexit_handler() {
    kill_all_child();
    puts("Kill all child process complete! Finish!");
}

void alarm_handler(int sig) {
    child_set_printAllowed_all();
}

void child_set_printAllowed_all() {
    for (size_t i = 0; i < count; ++i) {
        set_printAllowed(&child_array[i]);
    }
}

void set_printAllowed(child_t* child) {
    child->print_allowed = true;
}

void kill_all_child() {
    while (count > 0) {
        kill_last_child();
    }
}

pid_t kill_last_child() {
    if (count == 0) {
        fputs("Count child is 0", stderr);
        exit(EXIT_FAILURE);
    }
    send_signal(child_array[--count], PARENT_KILL);
    return wait(NULL);
}

void send_signal(child_t child, enum parent signal) {
    union sigval value = {signal};

    // associate value with SIGUSR1; send process with pid
    if (sigqueue(child.pid, SIGUSR1, value)) {
        perror("sigqueue");
        exit(errno);
    }
}

void child_handler(int sig, siginfo_t* info, void* context) {
    // siginfo_t - Information associated with a signal.
    size_t i = get_child_index_by_pid(info->si_pid);

    switch (info->si_int) {
        case CHILD_ASK:
            if (child_array[i].print_allowed) {
                send_signal(child_array[i], PARENT_RESPONSE);
            }
            break;

        case CHILD_INFORM:
            printf("%d informed parent\n", child_array[i].pid);
            break;

        default:
            fprintf(stderr,
                    "Unknown signal occurred in child_handler: %d\n",
                    info->si_int);
            exit(EXIT_FAILURE);
    }
}

size_t get_child_index_by_pid(pid_t pid) {
    for (size_t i = 0; i < count; ++i) {
        if (child_array[i].pid == pid) {
            return i;
        }
    }
    fprintf(stderr, "pid=%d is not child of the process\n", pid);
    exit(EXIT_FAILURE);
}

// -------------------- Init Functions --------------------

void create_child() {
    if (count == CHILD_COUNT - 1) {
        fputs("Child amount overflow", stderr);
        exit(EXIT_FAILURE);
    }

    pid_t ret = fork();
    switch (ret) {
        case 0:
        {
            // Child process
            char child_name[8];
            sprintf(child_name, "C_%zu", count);
            if (execl("./child", child_name, NULL) == -1) {
                perror("execl");
                exit(errno);
            }
        }
            break;

        case -1:
            perror("fork");
            exit(errno);

        default:
            // Parent process
            child_array[count].pid           = ret;
            child_array[count].print_allowed = true;
            ++count;
            break;
    }
}

bool check_num(size_t num) {
    if (num >= count) {
        fputs("Num is bigger than count child", stderr);
        return false;
    }
    return true;
}

bool is_number_in_stdin() {
    int  c         = getc(stdin);
    bool is_number = isdigit(c);
    ungetc(c, stdin);
    return is_number;
}

size_t read_number() {
    size_t num;
    if (scanf("%zu", &num) != 1) {
        fputs("Scanf read error", stderr);
        exit(EXIT_FAILURE);
    }
    return num;
}

void child_unset_printAllowed_all() {
    for (size_t i = 0; i < count; ++i) {
        unset_printAllowed(&child_array[i]);
    }
}

void unset_printAllowed(child_t* child) {
    child->print_allowed = false;
}

void force_print(child_t child) {
    send_signal(child, PARENT_FORCE_PRINT);
}