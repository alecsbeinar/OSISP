CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE

SRC = message/message.c main.c consumer/consumer.c producer/producer.c

src: $(SRC) 
		$(CC) $(CFLAGS) -o $@ $^
	
.PHONY: run
run:
	make src

.PHONY: clean

clean:
	rm -rf src