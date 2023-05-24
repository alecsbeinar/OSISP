#include "producer.h"
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>


extern msg_struct *queue;
extern sem_t *mutex;
extern sem_t *added;
extern sem_t *extracted;
extern pid_t producers[];
extern int producers_amount;

void create_producer() {
    if (producers_amount == CHILD_MAX - 1) {
        fputs("Max value of producers\n", stderr);
        return;
    }

    switch (producers[producers_amount] = fork()) {
        default:
            // Parent process
            ++producers_amount;
            return;

        case 0:
            // Child process
            srand(getpid()); // send id to rand buffer
            break;

        case -1:
            perror("fork");
            exit(errno);
    }

    // Child process
    msg_t msg;
    int counter;
    while (1) {
        produce(&msg);

        sem_wait(extracted);

        sem_wait(mutex);
        counter = add_msg(&msg);
        sem_post(mutex);

        sem_post(added);

        printf("%d-p) %d produce msg: hash=%X\n",
               counter, getpid(), msg.hash);

        sleep(3);
    }
}

void produce(msg_t *msg) {
    int value = rand() % 257;
    if (value == 256) {
        msg->type = -1;
    } else {
        msg->type = 0;
        msg->size = value;
    }

    for (int i = 0; i < value; ++i) {
        msg->data[i] = (char) (rand() % 256);
    }

    msg->hash = hash(msg);
}


void remove_producer() {
    if (producers_amount == 0) {
        fputs("Amount producers = 0\n", stderr);
        return;
    }

    --producers_amount;
    kill(producers[producers_amount], SIGKILL);
    wait(NULL);
}
