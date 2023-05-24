#ifndef LAB04_CONSUMER_H
#define LAB04_CONSUMER_H

#include "../message/message.h"

void create_consumer();
void consume(msg_t *msg);
void remove_consumer();

#endif //LAB04_CONSUMER_H
