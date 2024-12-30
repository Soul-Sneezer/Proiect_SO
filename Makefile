CC = gcc
CFLAGS = -Wall -Wextra -g -O0

SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

HDRS = $(wildcard *.h)

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
