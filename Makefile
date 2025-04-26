# Compiler
CC = gcc

# Flags for the main program
MAIN_CFLAGS = -Wall -Wextra -I./src -I./src/common -I./src/utils -I./src/validation -I./src/database -I./src/main -I./src/transaction

# Flags for the admin program
ADMIN_CFLAGS = -Wall -Wextra -g -I./src -I./src/Admin -I./src/common -I./src/utils -I./src/transaction

# Source files for the main program
MAIN_SRC = src/main/main.c src/main/menu.c src/validation/card_num_validation.c src/database/database.c src/validation/pin_validation.c src/utils/logger.c src/common/utils.c src/Admin/admin.c src/transaction/transaction_manager.c src/database/customer_profile.c
# Explicitly list all object files for the main program
MAIN_OBJ = src/main/main.o src/main/menu.o src/validation/card_num_validation.o src/database/database.o src/validation/pin_validation.o src/utils/logger.o src/common/utils.o src/Admin/admin.o src/transaction/transaction_manager.o src/database/customer_profile.o
MAIN_EXEC = run

# Source files for the admin program
ADMIN_SRC = src/Admin/admin.c src/Admin/admin_db.c src/Admin/admin_operations.c src/transaction/transaction_manager.c src/database/customer_profile.c
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
	-del /f /q $(MAIN_OBJ) $(MAIN_EXEC) $(ADMIN_OBJ) $(ADMIN_EXEC) 2>nul
	-if exist $(MAIN_EXEC) del /f /q $(MAIN_EXEC)
	-if exist $(ADMIN_EXEC) del /f /q $(ADMIN_EXEC)

.PHONY: all clean
