#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include "message/message.h"
#include "producer/producer.h"
#include "consumer/consumer.h"


void atexit_handler();
void initialization();
void info();

const char *menu = "MENU:\n"
                          "1 - print options\n"
                          "2 - create producer\n"
                          "3 - delete producer\n"
                          "4 - create consumer\n"
                          "5 - delete consumer\n"
                          "6 - info\n"
                          "+ - increase queue\n"
                          "- - decrease queue\n"
                          "7 - quit";

msg_struct* queue; // ring buffer of messages

pthread_mutex_t mutex;

// SEMAPHORES
sem_t added;
sem_t extracted;

// Processes info
pthread_t producers[CHILD_MAX];
int producers_amount;
pthread_t consumers[CHILD_MAX];
int consumers_amount;

msg_t msg;
int counter;
int main() {
    initialization();

    puts(menu);
    while (1) {
        switch (getchar()) {
            case '1':
                puts(menu);
                break;

            case '2':
                create_producer();
                break;

            case '3':
                remove_producer();
                break;

            case '4':
                create_consumer();
                break;

            case '5':
                remove_consumer();
                break;

            case '6':
                info();
                break;

            case '+':
                sem_wait(&extracted);
                produce(&msg);
                pthread_mutex_lock(&mutex);
                counter = add_msg(&msg);
                pthread_mutex_unlock(&mutex);
                sem_post(&added);

                printf("%d) User produce msg: hash=%X\n",
                        counter, msg.hash);

                break;

            case '-':
                if(queue->message_amount == 0){
                    printf("There are no messages in the queue\n");
                    break;
                }
                sem_wait(&added);
                pthread_mutex_lock(&mutex);
                counter = get_msg(&msg);
                pthread_mutex_unlock(&mutex);
                sem_post(&extracted);

                consume(&msg);

                printf("%d) User consume msg: hash=%X\n",
                       counter, msg.hash);
                break;

            case '7':
                exit(EXIT_SUCCESS);

            case EOF:
                fputs("Input error", stderr);
                exit(EXIT_FAILURE);

            default:
                break;
        }
    }
    return 0;
}


void initialization() {
    atexit(atexit_handler);

    // Setup shared memory
    // O_RDWR - read and write
    // 0_CREAT - creat object if it doesn't exist
    // O_TRUNC - fresh existed object
    // SHM_OBJECT - name POSIX shared memory object, '/' in start
    int file_descriptor = shm_open(SHARED_MEMORY_OBJ, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_descriptor < 0) {
        perror("shm_open");
        exit(errno);
    }

    // set length of file
    if (ftruncate(file_descriptor, sizeof(msg_struct))) {
        perror("ftruncate");
        exit(errno);
    }

    // copy bytes from file description into memory
    // PROT_* set rules for this part of memory
    // MAP_SHARED - flags set allowing for other processes to use
    // return pointer to memory with shared data
    void *ptr = mmap(NULL, sizeof(msg_struct), PROT_READ | PROT_WRITE,
                     MAP_SHARED, file_descriptor, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(errno);
    }

    queue = (msg_struct*) ptr;

    // Setup queue
    init_queue();


    // Setup mutex
    int res = pthread_mutex_init(&mutex, NULL);
    if(res){
        fputs("Failed mutex init\n", stderr);
        exit(res);
    }

    // Setup semaphores
    // set name, creat flags, read/write mode, default value
    // extracted - default value = MSG_MAX - when producer start work, it decrements this count and add message to queue (MSG_MAX - max messages in queue)
    // added - default value = 0 - consumer can't get messages, without start producer
    if (sem_init(&extracted, 0, MSG_MAX) || sem_init(&added, 0, 0))
    {
        perror("sem_init");
        exit(errno);
    }

    if (close(file_descriptor)) {
        perror("close");
        exit(errno);
    }
}

void atexit_handler() {
    int res = pthread_mutex_destroy(&mutex);
    if (res) {
        fputs("Failed mutex destroy\n", stderr);
        exit(res);
    }

    // remove shared memory segment
    if (shm_unlink(SHARED_MEMORY_OBJ)) {
        perror("shm_unlink");
        abort();
    }
    // remove named semaphore
    if (sem_destroy(&extracted) || sem_destroy(&added)) {
        perror("sem_destroy");
        abort();
    }
}


void info(){
    pthread_mutex_lock(&mutex);
    printf("Count producer: %d; count consumer: %d; count messages in queue: %d\n", producers_amount, consumers_amount, queue->message_amount);
    pthread_mutex_unlock(&mutex);
}
