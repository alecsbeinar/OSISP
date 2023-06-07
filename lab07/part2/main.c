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
    if (lseek(fd, record_num * RECORD_SIZE, SEEK_SET) == -1) {
        return -1;
    }
    if (read(fd, record, sizeof(struct record_s)) <= 0) {
        return -1;
    }
    return 0;
}

void list_records(int fd) {
    struct record_s record;
    printf("Records:\n");
    for (int i = 0; i < NUM_RECORDS; i++) {
        if (!get_record(fd, i, &record))
            printf("%d. %s, %s, %d\n", i, record.name, record.address, record.semester);
        else return;
    }
}

void put_record(int fd, int record_num, struct record_s *record) {
    if (lseek(fd, record_num * RECORD_SIZE, SEEK_SET) == -1) {
        perror("lseek");
        return;
    }
    if (write(fd, record, sizeof(struct record_s)) == -1) {
        perror("write");
        return;
    }
}

void modify_record(struct record_s *record) {
    char input[80];
    printf("Enter name: ");
    if (!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    strncpy(record->name, input, 80);
    printf("Enter address: ");
    if (!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    strncpy(record->address, input, 80);
    printf("Enter semester: ");
    if (!fgets(input, 80, stdin)) printf("Error: fgets\n");
    input[strcspn(input, "\n")] = 0;
    record->semester = strtol(input, NULL, 10);
}

void print_record(struct record_s record, int record_num) {
    printf("Record #%d\n", record_num);
    printf("Name: %s\n", record.name);
    printf("Address: %s\n", record.address);
    printf("Semester: %d\n", record.semester);
}

bool cmp_record(struct record_s record1, struct record_s record2) {
    return !strcmp(record1.name, record2.name) && !strcmp(record1.address, record2.address) &&
           record1.semester == record2.semester;
}

void add_record(int fd){
    int add_num = -1;
    struct record_s added_rec;
    for (int i = 0; i < NUM_RECORDS; i++) {
        if (get_record(fd, i, &added_rec)) {
            add_num = i;
            break;
        }
    }

    if (add_num == -1) {
        printf("No free records\n");
    } else if (get_lock(fd, add_num * RECORD_SIZE, SEEK_SET, F_WRLCK)) {
        printf("Record is locked\n");
    } else {
        memset(&added_rec, 0, sizeof(struct record_s));
        modify_record(&added_rec);
        put_record(fd, add_num, &added_rec);
        unlock_record(fd, add_num * RECORD_SIZE, SEEK_SET);
        printf("Record added at position %d\n", add_num);
    }
}

int main() {
    int fd = open("records.txt", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
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
    struct record_s save_record;

    while (1) {
        printf("> ");
        rewind(stdin);
        if (!fgets(input, 80, stdin)) {
            printf("Error: fgets\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "LST") == 0) {
            list_records(fd);
        } else if (strcmp(input, "ADD") == 0) {
            add_record(fd);
        } else if (strncmp(input, "MOD", 3) == 0) {
            if (record_num == -1) {
                printf("Nothing to edit. Call GET function!\n");
            } else {
                save_record = record;
                modify_record(&record);
            }
        } else if (strncmp(input, "GET ", 4) == 0 && strlen(input) > 4) {
            char* endptr;
            record_num = (int) strtol(input + 4, &endptr, 10);
            if(endptr[0] != '\0' || get_record(fd, record_num, &record) == -1){
                printf("Invalid number\n");
                record_num = -1;
            } else {
                print_record(record, record_num);
            }
        } else if (strcmp(input, "PUT") == 0) {
            struct record_s curr_rec;
            AGAIN:
            get_record(fd, record_num, &curr_rec);
            if (record_num == -1) {
                printf("No record to save\n");
            } else if(cmp_record(curr_rec, record)) {
                printf("To save, you need to change the data\n");
            } else if (get_lock(fd, record_num * RECORD_SIZE, SEEK_SET, F_WRLCK)) {
                printf("Record is locked\n");
            } else {
                if(cmp_record(save_record, curr_rec)) {
                    put_record(fd, record_num, &record);
                    save_record = record;
                    unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                    printf("Record #%d saved\n", record_num);
                } else {
                    unlock_record(fd, record_num * RECORD_SIZE, SEEK_SET);
                    printf("Data has been updated, please try again!\n");
                    
                    save_record = curr_rec;
                    record = curr_rec;
                    print_record(record, record_num);
                    printf("Modify new record? (y - yes, n - no)\n");
                    if(getchar() == 'y'){
                        while ((getchar()) != '\n');
                        modify_record(&record);
                        goto AGAIN;
                    }
                    while ((getchar()) != '\n');
                }
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