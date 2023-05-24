CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE

GENERATE = ./generate_unsorted_index.c
SORT = ./sort_index.c
PRINT = ./print_index.c


generate_unsorted_index: $(GENERATE) 
		$(CC) $(CFLAGS) -o $@ $^

sort_index: $(SORT)
		$(CC) $(CFLAGS) -o $@ $^
		
print_index: $(PRINT)
		$(CC) $(CFLAGS) -o $@ $^

	
.PHONY: run
run:
	make generate_unsorted_index
	make sort_index
	make print_index
.PHONY: clean

clean:
	rm -rf generate_unsorted_index sort_index print_index file.bin sorted_data.bin
