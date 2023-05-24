CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE

PART1 = part1/message/message.c part1/main.c part1/consumer/consumer.c part1/producer/producer.c
PART2 = part2/main.c

cond: $(PART1) 
		$(CC) $(CFLAGS) -o $@ $^
		
record: $(PART2) 
		$(CC) $(CFLAGS) -o $@ $^
	
.PHONY: run
run:
	make cond
	make record

.PHONY: clean

clean:
	rm -rf cond record
