CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE
OBJFILES = message/message.c main.c consumer/consumer.c producer/producer.c message/message.h consumer/consumer.h producer/producer.h
TARGET = src

all: $(TARGET)
	
$(TARGET): $(OBJFILES)
			$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

.PHONY: run
run:
	make all
.PHONY: clean

clean:
	rm -rf $(TARGET)