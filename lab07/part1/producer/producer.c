#include "producer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


extern msg_struct *queue;
extern pthread_mutex_t mutexp;
extern pthread_cond_t condp;
extern pthread_cond_t condc;
extern pthread_t producers[];
extern int producers_amount;

void create_producer() {
    if (producers_amount == CHILD_MAX - 1) {
        fputs("Max value of producers\n", stderr);
        return;
    }

    int res = pthread_create(&producers[producers_amount], NULL, produce_handler, NULL);
    if (res) {
        fputs("Failed to create producer\n", stderr);
        exit(res);
    }

    ++producers_amount;
}

void cleanup_handler(void *plock) {
    pthread_mutex_unlock(plock);
}

_Noreturn void *produce_handler(void *arg) {
    pthread_cleanup_push(cleanup_handler, &mutexp) ;

            msg_t msg;
            int counter;
            while (1) {
                produce(&msg);
                pthread_mutex_lock(&mutexp);
                while (queue->message_amount == MSG_MAX - 1) {
                    pthread_cond_wait(&condp, &mutexp);
                }

                counter = add_msg(&msg);

                if (counter != -1) {
                    pthread_cond_signal(&condc);
                    pthread_mutex_unlock(&mutexp);
                    pthread_t ptid = pthread_self();
                    printf("%d-p) %ld produce msg: hash=%X\n",
                           counter, ptid, msg.hash);
                } else
                    pthread_mutex_unlock(&mutexp);

                sleep(3);
            }

    pthread_cleanup_pop(0);
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
    // terminate thread
    pthread_cancel(producers[producers_amount]);
    // wait terminating
    pthread_join(producers[producers_amount], NULL);
}
