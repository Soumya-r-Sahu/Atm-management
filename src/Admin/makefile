# Makefile for admin.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Source and target
SRC = admin.c
OBJ = admin.o
TARGET = admin

# Build target
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile source file
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)