#include <bits/types/FILE.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint-gcc.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s num_records filename\n", argv[0]);
        return 1;
    }
    // Получение параметров командной строки
    int num_records = (int) strtol(argv[1], NULL, 10);
    if (num_records % 256 != 0) {
        fprintf(stderr, "Index size in records must be a multiple of 256\n");
        return 1;
    }
    int page_size = getpagesize();
    if (num_records < page_size || num_records % page_size != 0) {
        fprintf(stderr,
                "The size of the index in records must be a multiple of the planned allocated memory for mapping (%d)\n",
                page_size);
        return 1;
    }
    char *file_name = argv[2];

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("Failed to create index file");
        exit(EXIT_FAILURE);
    }

    // Запись заголовка
    struct index_hdr_s header;
    header.records = num_records;
    fwrite(&header, sizeof(struct index_hdr_s), 1, file);

    // Запись индексных записей
    // Инициализация генератора случайных чисел с текущим временем
    srandom(time(NULL));
    for (int i = 0; i < num_records; i++) {
        // Генерация случайной целой части в диапазоне от 15020 до вчерашнего дня - 1
        int max_integer_part = 15020;
        time_t current_time = time(NULL);
        time_t yesterday_integer_part = current_time / (24 * 60 * 60) - 1;
        long random_integer_part = max_integer_part + random() % (yesterday_integer_part - max_integer_part + 1);
        // Генерация случайной дробной части от 0.5 до 0.999999
        double random_fractional_part = 0.5 + (double) random() / ((double) RAND_MAX + 1);
        // Суммирование целой и дробной частей для получения временной метки
        double modified_jd = (double) random_integer_part + random_fractional_part;

        struct index_s record;
        record.time_mark = modified_jd;
        record.recno = i + 1;
        fwrite(&record, sizeof(struct index_s), 1, file);
    }

    fclose(file);
}