CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall

all:	writer

writer:	writer.c
	$(CC) $(CFLAGS) -o writer writer.c
	
clean:
	rm -f *.o writer
