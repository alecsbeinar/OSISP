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

pthread_mutex_t mutexp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexc = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condp = PTHREAD_COND_INITIALIZER;
pthread_cond_t condc = PTHREAD_COND_INITIALIZER;


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
                if (queue->message_amount == MSG_MAX - 1) {
                    fputs("Queue buffer overflow\n", stderr);
                } else {
                    produce(&msg);
                    pthread_mutex_lock(&mutexp);
                    while (queue->message_amount == MSG_MAX - 1)
                        pthread_cond_wait(&condp, &mutexp);
                    counter = add_msg(&msg);
                    pthread_cond_signal(&condc);
                    pthread_mutex_unlock(&mutexp);

                    printf("%d) User produce msg: hash=%X\n",
                           counter, msg.hash);
                }
                break;

            case '-':
                if(queue->message_amount == 0){
                    printf("There are no messages in the queue\n");
                    break;
                }
                pthread_mutex_lock(&mutexc);
                while (queue->message_amount == 0)
                    pthread_cond_wait(&condc, &mutexc);

                counter = get_msg(&msg);

                pthread_cond_signal(&condp);
                pthread_mutex_unlock(&mutexc);

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
    int res = pthread_mutex_init(&mutexp, NULL);
    if(res){
        fputs("Failed mutex init\n", stderr);
        exit(res);
    }
    res = pthread_mutex_init(&mutexc, NULL);
    if(res){
        fputs("Failed mutex init\n", stderr);
        exit(res);
    }

    pthread_cond_init(&condp, NULL);
    pthread_cond_init(&condc, NULL);

    if (close(file_descriptor)) {
        perror("close");
        exit(errno);
    }
}

void atexit_handler() {
    while(producers_amount > 0) remove_producer();
    while(consumers_amount > 0) remove_consumer();

    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condp);
    if(pthread_cond_destroy(&condp) != 0){ fputs("Failed cond destroy\n", stderr);  }
    if(pthread_cond_destroy(&condc) != 0){ fputs("Failed cond destroy\n", stderr); }

    pthread_mutex_destroy(&mutexp);
    pthread_mutex_destroy(&mutexc);

    // remove shared memory segment
    if (shm_unlink(SHARED_MEMORY_OBJ)) {
        perror("shm_unlink");
        abort();
    }

}


void info(){
    printf("Count producer: %d; count consumer: %d; count messages in queue: %d\n", producers_amount, consumers_amount, queue->message_amount);
}
