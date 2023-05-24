CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE

_CHILD = main.c child.c
CHILD = $(patsubst %,chld/%,$(_CHILD))

_PARENT = main.c parent.c
PARENT = $(patsubst %,prnt/%,$(_PARENT))

.PHONY: run
run:
	make child
	make parent
.PHONY: clean


child: $(CHILD)
	$(CC) $(CFLAGS) -o $@ $^

parent: $(PARENT)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f child parent
