#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint-gcc.h>
#include <stdbool.h>

#define RECORD_SIZE 168
#define NUM_RECORDS 15

struct record_s {
    char name[80];
    char address[80];
    uint8_t semester;
};

// set write lock, otherwise set lock.l_type in F_UNLCK
int get_lock(int fd, int offset, short whence, short type) {
    struct flock lock;
    lock.l_type = type; // F_WRLCK
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = RECORD_SIZE;
    if (fcntl(fd, F_GETLK, &lock) == -1) {
        perror("fcntl");
        exit(1);
    }
    return lock.l_type != F_UNLCK;
}

void unlock_record(int fd, int offset, short whence) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = RECORD_SIZE;
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl");
        exit(1);
    }
}

int get_record(int fd, int record_num, struct record_s *record) {
    if(lseek(fd, record_num * RECORD_SIZE, SEEK_SET) == -1){
        return -1;
    }
    if(read(fd, record, sizeof(struct record_s)) <= 0){
        return -1;
    }
    return 0;
}

void list_records(int fd) {
    struct record_s record;
    printf("Records:\n");
    for (int i = 0; i < NUM_RECORDS; i++) {
        if(!get_record(fd, i, &record))
            printf("%d. %s, %s, %d\n", i, record.name, record.address, record.semester);
        else return;
    }
}

void put_record(int fd, int record_num, struct record_s *record) {
    if(lseek(fd, record_num * RECORD_SIZE, SEEK_SET) == -1){
        perror("lseek");
        return;
    }
    if(write(fd, record, sizeof(struct record_s)) == -1){
        perror("write");
        return;
    }
}

void modify_record(struct record_s *record) {
    char input[80];
    printf("Enter name: ");
    if(!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    strncpy(record->name, input, 80);
    printf("Enter address: ");
    if(!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    strncpy(record->address, input, 80);
    printf("Enter semester: ");
    if(!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    record->semester = strtol(input, NULL, 10);
}

void print_record(struct record_s record, int record_num) {
    printf("Record #%d\n", record_num);
    printf("Name: %s\n", record.name);
    printf("Address: %s\n", record.address);
    printf("Semester: %d\n", record.semester);
    printf("\n");
}

bool cmp_record(struct record_s record1, struct record_s record2){
    return !strcmp(record1.name, record2.name) && !strcmp(record1.address, record2.address) && record1.semester == record2.semester;
}

int main() {
    int fd = open("records.txt", O_RDWR | O_CREAT,  S_IRWXU | S_IRWXG |  S_IRWXO);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    printf("Commands:\n");
    printf("ADD - add a new record\n");
    printf("LST - list all records\n");
    printf("GET <record number> - get record with number n\n");
    printf("MOD - modify an existing record\n");
    printf("PUT - save modified record\n");
    printf("QUIT - exit the program\n");

    char input[80];
    int record_num = -1;
    struct record_s record;
    while (1) {
        printf("> ");
        if(!fgets(input, 80, stdin)) printf("Error: fgets\n");
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "LST") == 0) {
            list_records(fd);
        } else if (strcmp(input, "ADD") == 0) {
            record_num = -1;
            for (int i = 0; i < NUM_RECORDS; i++) {
                // !get_lock(fd, i * RECORD_SIZE, SEEK_SET, F_WRLCK)
                if (get_record(fd, i, &record)) {
                    record_num = i;
                    break;
                }
            }
            if (record_num == -1) {
                printf("No free records\n");
            } else if (get_lock(fd, record_num * RECORD_SIZE, SEEK_SET, F_WRLCK)) {
                printf("Record is locked\n");
            } else {
                memset(&record, 0, sizeof(struct record_s));
                modify_record(&record);
                put_record(fd, record_num, &record);
                unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                printf("Record added at position %d\n", record_num);
            }
        } else if (strncmp(input, "MOD", 3) == 0) {
            if(record_num == -1){
                printf("Nothing to edit. Call GET function!\n");
            } else {
                struct record_s record_sav;
                AGAIN:
                record_sav = record;
                modify_record(&record);
                if(!cmp_record(record, record_sav)){
                    if (get_lock(fd, record_num * RECORD_SIZE, SEEK_SET, F_WRLCK)) {
                        printf("Record is locked\n");
                    } else {
                        struct record_s record_new;
                        get_record(fd, record_num, &record_new);
                        if(cmp_record(record_sav, record_new)){
                            // одинаковые (никто не изменил пока мы думали)
                            // put_record(fd, record_num, &record);
                            unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                            printf("Record modified at position %d\n", record_num);
                        }
                        else{
                            // кто-то изменил запись после получения ее нами
                            // повторяем с ее новым содержимым
                            printf("Data has been updated, please try again!\n");
                            unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                            record = record_new;
                            goto AGAIN;
                        }
                    }
                }
            }
        } else if (strncmp(input, "GET ", 4) == 0) {
            record_num = (int) strtol(input + 4, NULL, 10);
            get_record(fd, record_num, &record);
            print_record(record, record_num);
        } else if (strcmp(input, "PUT") == 0) {
            if (record_num == -1) {
                printf("No record to save\n");
            } else if (get_lock(fd, record_num * RECORD_SIZE, SEEK_SET, F_WRLCK)) {
                printf("Record is locked\n");
            } else {
                put_record(fd, record_num, &record);
                unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                printf("Record #%d saved\n", record_num);
            }
        } else if (strcmp(input, "QUIT") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    close(fd);
    return 0;
}
