#include "consumer.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>


extern msg_struct *queue;
extern sem_t *mutex;
extern sem_t *added;
extern sem_t *extracted;
extern pid_t consumers[];
extern int consumers_amount;

void create_consumer(){
    if (consumers_amount == CHILD_MAX - 1) {
        fputs("Max value of consumers\n", stderr);
        return;
    }

    switch (consumers[consumers_amount] = fork()) {
        default:
            // Parent process
            ++consumers_amount;
            return;

        case 0:
            // Child process
            break;

        case -1:
            perror("fork");
            exit(errno);
    }

    // Child process
    msg_t msg;
    int counter;
    while(1){
        // decrement semaphore with name to 0; if at this moment 0 - waiting
        sem_wait(added);

        sem_wait(mutex);
        counter = get_msg(&msg);
        // increment semaphore with name
        sem_post(mutex);

        sem_post(extracted);

        consume(&msg);

        printf("%d-c) %d consume msg: hash=%X\n",
               counter, getpid(), msg.hash);

        sleep(3);
    }
}

void consume(msg_t *msg) {
    int control_hash = hash(msg);
    if (msg->hash != control_hash) {
        fprintf(stderr, "control_hash=%X not equal msg_hash=%X\n",
                control_hash, msg->hash);
    }
}

void remove_consumer(){
    if (consumers_amount == 0) {
        fputs("Amount consumers = 0\n", stderr);
        return;
    }

    --consumers_amount;
    kill(consumers[consumers_amount], SIGKILL);
    wait(NULL);
}
