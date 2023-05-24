#include "consumer.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>


extern msg_struct *queue;
extern pthread_mutex_t mutex;
extern pthread_cond_t condp;
extern pthread_cond_t condc;
extern pthread_t consumers[];
extern int consumers_amount;

void create_consumer(){
    if (consumers_amount == CHILD_MAX - 1) {
        fputs("Max value of consumers\n", stderr);
        return;
    }

    int res = pthread_create(&consumers[consumers_amount], NULL, consume_handler, NULL);
    if (res) {
        fputs("Failed to create producer\n", stderr);
        exit(res);
    }

    ++consumers_amount;
}

void* consume_handler(void* arg){
    msg_t msg;
    int counter;
    while(1){
        pthread_mutex_lock(&mutex);
        while (queue->message_amount == 0)
            pthread_cond_wait(&condc, &mutex);

        counter = get_msg(&msg);

        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&mutex);

        consume(&msg);

        pthread_t ptid = pthread_self();
        printf("%d-c) %ld consume msg: hash=%X\n",
               counter, ptid, msg.hash);

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
    pthread_cancel(consumers[consumers_amount]);
    pthread_join(consumers[consumers_amount], NULL);
}
