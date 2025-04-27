# Compiler
CC = gcc

# Project directories - use double quotes for Windows paths with spaces
DATA_DIR = "data"

# Common flags for all builds
CFLAGS = -Wall -Wextra -g -I./src -I./src/common -I./src/utils -I./src/validation -I./src/database -I./src/main -I./src/transaction -I./src/Admin -DDATA_DIR='$(DATA_DIR)'

# Source files for the single executable that includes all functionality
SRCS = src/main/main.c \
       src/validation/card_num_validation.c \
       src/validation/pin_validation.c \
       src/database/database.c \
       src/utils/logger.c \
       src/main/menu.c \
       src/common/paths.c \
       src/utils/language_support.c \
       src/config/config_manager.c \
       src/Admin/admin.c \
       src/Admin/admin_db.c \
       src/Admin/admin_menu.c \
       src/Admin/admin_operations.c \
       src/Admin/admin_interface.c \
       src/transaction/transaction_manager.c \
       src/database/customer_profile.c \
       src/database/database_utils.c \
       src/utils/file_utils.c \
       src/utils/hash_utils.c \
       src/utils/string_utils.c \
       src/common/utils.c

# Object files
OBJS = $(SRCS:.c=.o)

# Final executable name
EXEC = atm_system

# Default target: build the single executable
all: $(EXEC)

# Build the unified executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)

# Dependency rule
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
