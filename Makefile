CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = \
    ./dir.c \
	./main.c 

HDRS = \
    ./dir.h

TARGET = main

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)