CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE
TARGET1 = generate_unsorted_index
OBJFILES1 = generate_unsorted_index.c
TARGET2 = sort_index
OBJFILES2 = sort_index.c
TARGET3 = print_index
OBJFILES3 = print_index.c

all: $(TARGET1) $(TARGET2) $(TARGET3)
	
$(TARGET1): $(OBJFILES1)
			$(CC) $(CFLAGS) -o $(TARGET1) $(OBJFILES1)

$(TARGET2): $(OBJFILES2)
			$(CC) $(CFLAGS) -o $(TARGET2) $(OBJFILES2)

$(TARGET3): $(OBJFILES3)
			$(CC) $(CFLAGS) -o $(TARGET3) $(OBJFILES3)

.PHONY: run
run:
	make all
.PHONY: clean

clean:
	rm -rf $(TARGET1) $(TARGET2) $(TARGET3) file.bin sorted_data.bin