CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE
TARGET1 = cond
OBJFILES1 = part1/message/message.c part1/main.c part1/consumer/consumer.c part1/producer/producer.c part1/message/message.h part1/consumer/consumer.h part1/producer/producer.h
TARGET2 = record
OBJFILES2 = part2/main.c


all: $(TARGET1) $(TARGET2)
	
$(TARGET1): $(OBJFILES1)
			$(CC) $(CFLAGS) -o $(TARGET1) $(OBJFILES1)

$(TARGET2): $(OBJFILES2)
			$(CC) $(CFLAGS) -o $(TARGET2) $(OBJFILES2)

.PHONY: run
run:
	make all
.PHONY: clean

clean:
	rm -rf $(TARGET1) $(TARGET2)