CC = gcc
CFLAGS = -O2 -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE

TARGET1 = srvr
OBJFILES1 = server/main.c
TARGET2 = clnt
OBJFILES2 = client/main.c


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