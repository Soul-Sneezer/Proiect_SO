# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Source files
SRCS = \
	./main.c \
    ./telemetry.c \
	./error_functions.c 

# Header files
HDRS = \
    ./error_functions.h \
    ./tlpi_hdr.h \
    ./telemetry.h

# Output executable
TARGET = telemetry

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(TARGET)

# Clean up
clean:
	rm -f $(TARGET)
