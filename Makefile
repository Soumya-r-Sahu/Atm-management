# Compiler
CC = gcc

# Project directories - use double quotes for Windows paths with spaces
DATA_DIR = "data"

# Common flags for all builds
CFLAGS = -Wall -Wextra -g -I./src -I./src/common -I./src/utils -I./src/validation -I./src/database -I./src/main -I./src/transaction -I./src/admin -I./include -I./include/common -I./include/atm -I./include/admin -I./include/upi_transaction -DDATA_DIR='$(DATA_DIR)'

# Source files for ATM executable
ATM_SRCS = src/atm/atm_main.c \
           src/atm/validation/card_validator.c \
           src/atm/transaction/transaction_processor.c \
           src/atm/transaction/transaction_utils.c \
           src/atm/ui/atm_menu.c \
           src/common/utils/logger.c \
           src/common/config/config_manager.c \
           src/common/utils/hash_utils.c \
           src/database/database.c \
           src/common/utils/path_manager.c

# Source files for Admin executable
ADMIN_SRCS = src/admin/admin_main.c \
             src/admin/management/admin_operations.c \
             src/admin/ui/admin_menu.c \
             src/admin/auth/admin_auth.c \
             src/admin/auth/admin_db.c \
             src/common/utils/logger.c \
             src/database/database.c \
             src/common/utils/path_manager.c

# Source files for UPI executable
UPI_SRCS = src/upi_transaction/upi_transaction.c \
           src/common/utils/file_utils.c \
           src/common/utils/string_utils.c \
           src/common/security/hash_utils.c \
           src/common/utils/logger.c \
           src/common/database/database.c

# Object files
ATM_OBJS = $(ATM_SRCS:.c=.o)
ADMIN_OBJS = $(ADMIN_SRCS:.c=.o)
UPI_OBJS = $(UPI_SRCS:.c=.o)

# Final executable names
ATM_EXEC = atm_system
ADMIN_EXEC = admin_system
UPI_EXEC = upi_system

# Default target: build all executables
all: $(ATM_EXEC) $(ADMIN_EXEC) $(UPI_EXEC)

# Build the ATM executable
$(ATM_EXEC): $(ATM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ATM_OBJS) -lm -lc

# Build the Admin executable
$(ADMIN_EXEC): $(ADMIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ADMIN_OBJS) -lm -lc

# Build the UPI executable
$(UPI_EXEC): $(UPI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UPI_OBJS) -lm -lc

# Clean up
clean:
	rm -f $(ATM_OBJS) $(ADMIN_OBJS) $(UPI_OBJS) $(ATM_EXEC) $(ADMIN_EXEC) $(UPI_EXEC)

# Dependency rule
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
