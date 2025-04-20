# Compiler
CC = gcc

# Flags for the main program
MAIN_CFLAGS = -Wall -Wextra -I./src

# Flags for the admin program
ADMIN_CFLAGS = -Wall -Wextra -g -I./src

# Source files for the main program
MAIN_SRC = src/main.c src/card_num_validation.c src/database.c src/pin_validation.c src/utils/logger.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
MAIN_EXEC = run

# Source files for the admin program
ADMIN_SRC = src/Admin/admin.c
ADMIN_OBJ = $(ADMIN_SRC:.c=.o)
ADMIN_EXEC = src/Admin/admin

# Default target: build both programs
all: $(MAIN_EXEC) $(ADMIN_EXEC)

# Build the main program
$(MAIN_EXEC): $(MAIN_OBJ)
	$(CC) $(MAIN_CFLAGS) -o $@ $^

# Build the admin program
$(ADMIN_EXEC): $(ADMIN_OBJ)
	$(CC) $(ADMIN_CFLAGS) -o $@ $^

# Compile source files for the main program
%.o: %.c
	$(CC) $(MAIN_CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(MAIN_OBJ) $(MAIN_EXEC) $(ADMIN_OBJ) $(ADMIN_EXEC)

.PHONY: all clean
