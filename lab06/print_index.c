#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>

// Структура для индексной записи
struct index_s {
    double time_mark; // временная метка (модифицированная юлианская дата)
    uint64_t recno;   // первичный индекс в таблице БД
};

// Структура для заголовка
struct index_hdr_s {
    uint64_t records;       // количество записей
    struct index_s idx[];   // массив записей в количестве records
};

void print_file_without_header(char *);

void print_file_with_header(char *);


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s filename {0-with header, 1-without header}\n", argv[0]);
        return -1;
    }

    switch (strtol(argv[2], NULL, 10)) {
        case 0:
            print_file_with_header(argv[1]);
            break;
        case 1:
            print_file_without_header(argv[1]);
            break;
        default:
            fprintf(stderr, "Third argument cannot be specified\n");
            return -1;
    }

    getchar();
    return 0;
}


void print_file_with_header(char *file_name) {
    FILE *file = fopen(file_name, "rb");  // Открытие файла для чтения в бинарном режиме
    if (file == NULL) {
        perror("Failed to open file\n");
        return;
    }

    struct index_s index;
    struct index_hdr_s indexHdrS;
    size_t readSize;
    if ((readSize = fread(&indexHdrS, sizeof(struct index_hdr_s), 1, file)) > 0) {
        printf("Number of records: %lu\n", indexHdrS.records);
    }
    // Чтение и вывод содержимого файла
    while ((readSize = fread(&index, sizeof(struct index_s), 1, file)) > 0) {
        printf("Value: %f, Index: %lu\n", index.time_mark, index.recno);
    }

    fclose(file);  // Закрытие файла
}

void print_file_without_header(char *file_name) {
    FILE *file = fopen(file_name, "rb");  // Открытие файла для чтения в бинарном режиме
    if (file == NULL) {
        perror("Failed to open file\n");
        return;
    }

    struct index_s index;
    size_t readSize;
    long counter = 0;
    while ((readSize = fread(&index, sizeof(struct index_s), 1, file)) > 0) {
        printf("Value: %f, Index: %lu\n", index.time_mark, index.recno);
        ++counter;
    }
    printf("Number of records: %lu\n", counter);

    fclose(file);  // Закрытие файла
}
