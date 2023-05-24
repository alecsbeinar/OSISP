CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter

CHILD = child/child.c

PARENT = main.c parent/parent.c

all:
	make chld
	make prnt

chld: $(CHILD)
	$(CC) $(CFLAGS) -o $@ $^

prnt: $(PARENT)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f chld prnt

