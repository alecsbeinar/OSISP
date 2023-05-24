#ifndef LAB04_PRODUCER_H
#define LAB04_PRODUCER_H

#include "../message/message.h"

void create_producer();
void* produce_handler(void* arg);
void produce(msg_t *msg);
void remove_producer();

#endif //LAB04_PRODUCER_H
