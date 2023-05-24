#ifndef LAB04_MESSAGE_H
#define LAB04_MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_MAX 4096
#define CHILD_MAX 1024
#define SHARED_MEMORY_OBJ "/queue"
#define MUTEX "mutex"
#define ADDED "added"
#define EXTRACTED "extracted"

typedef struct {
    int type;
    int hash;
    int size;
    char data[256 * 3 + 1];
} msg_t;

typedef struct {
    int produce_count;
    int consume_count;
    int message_amount;
    int head;
    int tail;
    msg_t buffer[MSG_MAX];
} msg_struct;

extern msg_struct* queue;

void init_queue();
int hash(msg_t*);
int add_msg(msg_t* msg); //return produce count
int get_msg(msg_t* msg); //return extract count

#endif //LAB04_MESSAGE_H
