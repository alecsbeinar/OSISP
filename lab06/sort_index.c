#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint-gcc.h>
#include <stdbool.h>
#include <memory.h>

#define BUFFER_FILENAME "buffer_data.bin"
#define SORTED_FILENAME "sorted_data.bin"
#define N 8192

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

// Глобальные переменные
int memsize;
int blocks;    // Количество блоков
int block_size;
long index_size;
int *busy_blocks;
int threads;
int count_sorted_memsize;
long records_in_memsize;
size_t count_indexes;

char *filename;
int file_fd;
void *file_map;  // Отображение файла в память
long file_size;  // Размер файла

pthread_mutex_t mutex; // Мьютекс для синхронизации доступа к карте блоков
pthread_barrier_t barrier;  // Барьер для синхронизации потоков


void *thread_func(void *);                          // Функция для потоков
int compare(const void *a, const void *b);          // Функция сравнения для qsort
void func_merge_blocks(long, long, struct index_s *);           // Функция слияния двух блоков
size_t cpy_file(const char *, const char *);

void merge_sorted_memsize(char *);


int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s memsize blocks threads filename\n", argv[0]);
        return -1;
    }

    // Получение параметров командной строки
    memsize = (int) strtol(argv[1], NULL, 10);
    blocks = (int) strtol(argv[2], NULL, 10);
    threads = (int) strtol(argv[3], NULL, 10);
    filename = argv[4];

    // Проверка валидности параметров
    int pagesize = getpagesize();
    if (memsize % pagesize != 0) {
        fprintf(stderr, "memsize should be a multiple of page size = %d\n", pagesize);
        return -1;
    }
    if ((blocks & (blocks - 1)) != 0) {
        fprintf(stderr, "blocks should be a power of 2\n");
        return -1;
    }
    if (blocks <= threads) {
        fprintf(stderr, "blocks should be greater then count of threads\n");
        return -1;
    }

    int core = sysconf(_SC_NPROCESSORS_ONLN);
    if (threads < core || threads > N) 
    {
        fprintf(stderr, "Amount of threads sholud be beetween %d и %d\n", core, N);
        return -1;
    }

    block_size = memsize / blocks;
    index_size = sizeof(struct index_s);
    count_sorted_memsize = 0;
    records_in_memsize = memsize / index_size;

    size_t control_value;
    control_value = cpy_file(filename, BUFFER_FILENAME);

    // Открытие файла
    file_fd = open(BUFFER_FILENAME, O_RDWR);
    if (file_fd == -1) {
        perror("Failed to open file");
        return -1;
    }
    // Получение размера файла
    file_size = lseek(file_fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Failed to get file size\n");
        close(file_fd);
        return -1;
    }
    count_indexes = file_size / index_size;
    if (control_value != count_indexes) {
        fprintf(stderr, "Error: discrepancy between the values in the source file and the actual number of records\n");
        return -1;
    }

    // Инициализация мьютекса и барьера
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barrier, NULL, threads);

    // Создание потоков
    pthread_t *thread_ids = (pthread_t *) malloc(threads * sizeof(pthread_t));
    int *thread_nums = (int *) malloc(threads * sizeof(int));
    for (int i = 0; i < threads; i++) {
        thread_nums[i] = i;
        pthread_create(&thread_ids[i], NULL, thread_func, &thread_nums[i]);
    }

    // Ожидание завершения потоков
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Освобождение ресурсов
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
    free(thread_ids);
    free(thread_nums);
    free(busy_blocks);

    // Закрытие файла
    close(file_fd);
    if (remove(BUFFER_FILENAME) == -1) {
        perror("Remove buffer_file");
        return -1;
    }
    return 0;
}

void *thread_func(void *arg) {
    int thread_num = *((int *) arg);

    STEP_2:
    if (thread_num == 0) {
        // Отображение файла в память
        file_map = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, memsize * count_sorted_memsize);
        if (file_map == MAP_FAILED) {
            perror("Failed to map file to memory\n");
            pthread_exit(NULL);
        }

        // Инициализация массива блоков и карты блоков
        busy_blocks = (int *) calloc(blocks, sizeof(int));
        for (int i = 0; i < blocks; i++) {
            busy_blocks[i] = i < threads ? i : -1;
        }
    }

    // ФАЗА СОРТИРОВКИ

    // Синхронизация потоков на барьере
    pthread_barrier_wait(&barrier);

    // Сортировка своего блока
    qsort(&(((struct index_s *) file_map)[thread_num * block_size / index_size]), block_size / index_size, index_size,
          compare);

    int block_id;
    while (true) {
        // Захват мьютекса для доступа к карте блоков
        pthread_mutex_lock(&mutex);

        // Поиск следующего свободного блока
        for (block_id = 0; block_id < blocks; block_id++) {
            if (busy_blocks[block_id] == -1) {
                // Блок найден, отмечаем его как занятый
                busy_blocks[block_id] = thread_num;
                break;
            }
        }

        // Освобождение мьютекса
        pthread_mutex_unlock(&mutex);

        // Если не найдено свободных блоков, выходим из цикла
        if (block_id == blocks) {
            break;
        }
        qsort(&(((struct index_s *) file_map)[block_id * block_size / index_size]), block_size / index_size, index_size,
              compare);
    }

    // Синхронизация потоков на барьере
    pthread_barrier_wait(&barrier);

    // ФАЗА СЛИЯНИЯ

    int merge_blocks = blocks;
    int merge_size = block_size;

    while (merge_blocks > 1) {
        memset(busy_blocks, -1, sizeof(int) * blocks);

        // Потоки, которым не достался блок для слияния, выходят из цикла и синхронизируются на барьере
        while (true) {
            // Захват мьютекса для доступа к карте блоков
            pthread_mutex_lock(&mutex);

            // Поиск следующего свободного блока
            for (block_id = 0; block_id < merge_blocks; block_id++) {
                if (busy_blocks[block_id] == -1) {
                    // Блоки найдены, отмечаем их как занятый
                    busy_blocks[block_id] = thread_num;
                    busy_blocks[block_id + 1] = thread_num;
                    break;
                }
            }

            // Освобождение мьютекса
            pthread_mutex_unlock(&mutex);

            // Если не найдено свободных блоков, выходим из цикла
            if (block_id == merge_blocks) {
                break;
            }

            long records_count = merge_size / index_size;

            pthread_mutex_lock(&mutex);
            func_merge_blocks(block_id * records_count, records_count, file_map);
            pthread_mutex_unlock(&mutex);
        }

        // Синхронизация на барьере после каждой фазы слияния
        pthread_barrier_wait(&barrier);

        // Обновление переменных для следующей итерации
        merge_blocks /= 2;
        merge_size *= 2;

    }

    // Сброс отсортированного буффера в файл

    if (thread_num == 0) {
        munmap(file_map, memsize);
        ++count_sorted_memsize;
    }

    // Синхронизация потоков на барьере
    pthread_barrier_wait(&barrier);
    if (count_sorted_memsize * memsize >= file_size) {
        if (thread_num == 0) {
            merge_sorted_memsize(SORTED_FILENAME);
        }
        pthread_exit(NULL);
    } else {
        goto STEP_2;
    }
}

int compare(const void *a, const void *b) {
    const struct index_s *index_a = (const struct index_s *) a;
    const struct index_s *index_b = (const struct index_s *) b;

    // Сравнение значений
    if (index_a->time_mark < index_b->time_mark) {
        return -1;  // index_a меньше index_b
    } else if (index_a->time_mark > index_b->time_mark) {
        return 1;  // index_a больше index_b
    } else {
        return 0;  // Значения равны
    }
}

void func_merge_blocks(long block_number, long records_count, struct index_s *block_data) {
    // Вычисление начальной позиции первого блока в массиве данных
    long first_block_index = block_number;

    // Вычисление начальной позиции второго блока в массиве данных
    long second_block_index = block_number + records_count;

    // Создание временного буфера для хранения слитых данных
    struct index_s *merged_data = (struct index_s *) malloc(2 * records_count * index_size);

    long i = first_block_index;        // Индекс для первого блока
    long j = second_block_index;       // Индекс для второго блока
    long l = 0;                        // Индекс для временного буфера

    // Слияние двух блоков в временный буфер
    while (i < second_block_index && j < second_block_index + records_count) {
        if (block_data[i].time_mark <= block_data[j].time_mark) {
            merged_data[l] = block_data[i];
            i++;
        } else {
            merged_data[l] = block_data[j];
            j++;
        }
        l++;
    }

    // Копирование оставшихся элементов из первого блока (если есть)
    while (i < second_block_index) {
        merged_data[l] = block_data[i];
        i++;
        l++;
    }

    // Копирование оставшихся элементов из второго блока (если есть)
    while (j < second_block_index + records_count) {
        merged_data[l] = block_data[j];
        j++;
        l++;
    }

    usleep(1);

    // Копирование слитых данных обратно в исходный блок
    for (int idx = 0; idx < 2 * records_count; idx++) {
        block_data[first_block_index + idx] = merged_data[idx];
    }
    // Освобождение временного буфера
    free(merged_data);
}

// Функция чтения и вывода файла со структурами index_s
size_t cpy_file(const char *source, const char *destination) {
    FILE *file = fopen(source, "rb");  // Открытие файла для чтения в бинарном режиме
    if (file == NULL) {
        perror("Failed to open file\n");
        return 0;
    }

    struct index_s index;
    struct index_hdr_s indexHdrS;
    size_t readSize;

    size_t count_records = 0;
    if ((readSize = fread(&indexHdrS, sizeof(struct index_hdr_s), 1, file)) > 0) {
        printf("Number of records: %lu\n", indexHdrS.records);
        count_records = indexHdrS.records;
    }

    // Открытие файла для записи
    FILE *file_w = fopen(destination, "w");
    if (file_w == NULL) {
        perror("Failed to open file\n");
        return 0;
    }

    // Чтение и вывод содержимого файла
    while ((readSize = fread(&index, sizeof(struct index_s), 1, file)) > 0) {
        fwrite(&index, sizeof(struct index_s), 1, file_w);
    }

    fclose(file);  // Закрытие файла
    fclose(file_w);

    return count_records;
}

void merge_sorted_memsize(char *file_name) {
    // Открытие файла для записи
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        perror("Failed to open file\n");
        pthread_exit(NULL);
    }

    // Отображение блочного файла в память
    file_map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    if (file_map == MAP_FAILED) {
        perror("Failed to map file to memory\n");
        pthread_exit(NULL);
    }

    int *indexes = (int *) calloc(count_sorted_memsize, sizeof(int));
    struct index_s minValue;
    int minIndex;
    struct index_s tempValue;
    size_t total = 0;
    while (total < count_indexes) {
        minIndex = -1;
        for (int i = 0; i < count_sorted_memsize; i++) {
            if (indexes[i] < 0)
                continue;
            tempValue = ((struct index_s *) file_map)[indexes[i] + i * records_in_memsize];
            if (minIndex == -1) {
                minValue = tempValue;
                minIndex = i;
            }
            if (tempValue.time_mark < minValue.time_mark) {
                minValue = tempValue;
                minIndex = i;
            }
        }
        fwrite(&minValue, index_size, 1, file);
        ++indexes[minIndex];
        ++total;
        if (indexes[minIndex] == records_in_memsize)
            indexes[minIndex] = -1;
    }

    munmap(file_map, file_size);
    fclose(file);
}


void print_file_with_header(char *file_name) {
    FILE *file = fopen(file_name, "rb");  // Открытие файла для чтения в бинарном режиме
    if (file == NULL) {
        printf("Не удалось открыть файл.\n");
        return;
    }

    struct index_s index;
    struct index_hdr_s indexHdrS;
    size_t readSize;
    if ((readSize = fread(&indexHdrS, sizeof(struct index_hdr_s), 1, file)) > 0) {
        printf("Количество записей: %lu\n", indexHdrS.records);
    }
    // Чтение и вывод содержимого файла
    while ((readSize = fread(&index, sizeof(struct index_s), 1, file)) > 0) {
        printf("Значение: %f, Индекс: %lu\n", index.time_mark, index.recno);
    }

    fclose(file);  // Закрытие файла
}


