CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -g

TARGET = aesdsocket
OBJS = aesdsocket.o

default: $(TARGET)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup target
clean:
	rm -f $(TARGET) $(OBJS)
