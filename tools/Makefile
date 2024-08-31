# Makefile for compiling test_strto.c

# Define variables
CC = gcc                      # Compiler to use
CFLAGS = -Wall -Wextra -O2   # Compiler flags
TARGET = test_strto           # Target executable name
SRC = test_strto.c            # Source file

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
