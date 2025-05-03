# ATM Management System Makefile

# Compiler
CC = gcc

# Project directories - Updated for reorganized structure
BACKEND_DIR = backend/c_backend
SRC_DIR = $(BACKEND_DIR)/src
INCLUDE_DIR = $(BACKEND_DIR)/include
BIN_DIR = bin
DATA_DIR = data
LOGS_DIR = logs
FRONTEND_CLI_DIR = frontend/cli

# Common flags for all builds
CFLAGS = -Wall -Wextra -g \
         -I$(INCLUDE_DIR) \
         -I$(INCLUDE_DIR)/common \
         -I$(INCLUDE_DIR)/atm \
         -I$(INCLUDE_DIR)/admin \
         -I$(INCLUDE_DIR)/upi \
         -I$(INCLUDE_DIR)/core \
         -I$(SRC_DIR)/common \
         -I$(FRONTEND_CLI_DIR)/include \
         -DDATA_DIR=\"$(DATA_DIR)\" \
         -DLOGS_DIR=\"$(LOGS_DIR)\"

# Add MySQL library to the linker flags
LDFLAGS += -lmysqlclient -lcrypto -lssl -lpthread

# Source files for ATM executable
ATM_SRCS = $(SRC_DIR)/atm/atm_main.c \
           $(SRC_DIR)/atm/auth/card_validator.c \
           $(SRC_DIR)/atm/transaction/transaction_processor.c \
           $(SRC_DIR)/atm/transaction/transaction_utils.c \
           $(SRC_DIR)/atm/ui/atm_menu.c \
           $(SRC_DIR)/common/utils/logger.c \
           $(SRC_DIR)/common/config/config_manager.c \
           $(SRC_DIR)/common/utils/hash_utils.c \
           $(SRC_DIR)/common/database/database.c \
           $(SRC_DIR)/common/utils/path_manager.c

# Source files for Admin executable
ADMIN_SRCS = $(SRC_DIR)/admin/admin_main.c \
             $(SRC_DIR)/admin/management/admin_operations.c \
             $(SRC_DIR)/admin/management/system/admin_system_manager.c \
             $(SRC_DIR)/admin/ui/admin_menu.c \
             $(SRC_DIR)/admin/auth/admin_auth.c \
             $(SRC_DIR)/admin/auth/admin_db.c \
             $(SRC_DIR)/common/database/card_management.c \
             $(SRC_DIR)/common/utils/logger.c \
             $(SRC_DIR)/common/utils/hash_utils.c \
             $(SRC_DIR)/common/database/database.c \
             $(SRC_DIR)/common/utils/path_manager.c

# Source files for UPI executable
UPI_SRCS = $(SRC_DIR)/upi_transaction/upi_transaction.c \
           $(SRC_DIR)/common/utils/file_utils.c \
           $(SRC_DIR)/common/utils/string_utils.c \
           $(SRC_DIR)/common/security/hash_utils.c \
           $(SRC_DIR)/common/utils/logger.c \
           $(SRC_DIR)/common/utils/path_manager.c \
           $(SRC_DIR)/common/database/database.c

# Source files for CLI Frontend
CLI_SRCS = $(FRONTEND_CLI_DIR)/src/main_menu.c \
           $(FRONTEND_CLI_DIR)/src/customer_menu.c \
           $(FRONTEND_CLI_DIR)/src/admin_menu.c \
           $(FRONTEND_CLI_DIR)/src/menu_utils.c

# Object files
ATM_OBJS = $(ATM_SRCS:.c=.o)
ADMIN_OBJS = $(ADMIN_SRCS:.c=.o)
UPI_OBJS = $(UPI_SRCS:.c=.o)
CLI_OBJS = $(CLI_SRCS:.c=.o)

# Final executable names
ATM_EXEC = $(BIN_DIR)/atm_system
ADMIN_EXEC = $(BIN_DIR)/admin_system
UPI_EXEC = $(BIN_DIR)/upi_system
CLI_EXEC = $(BIN_DIR)/atm_cli

# Default target: build all executables
all: create_dirs $(ATM_EXEC) $(ADMIN_EXEC) $(UPI_EXEC) $(CLI_EXEC)

# Create necessary directories
create_dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DATA_DIR)
	@mkdir -p $(LOGS_DIR)
	@mkdir -p $(DATA_DIR)/temp
	@mkdir -p $(SRC_DIR)/atm/auth
	@mkdir -p $(SRC_DIR)/atm/transaction
	@mkdir -p $(SRC_DIR)/atm/ui
	@mkdir -p $(SRC_DIR)/admin/management/system
	@mkdir -p $(SRC_DIR)/admin/ui
	@mkdir -p $(SRC_DIR)/admin/auth
	@mkdir -p $(SRC_DIR)/upi_transaction
	@mkdir -p $(SRC_DIR)/common/utils
	@mkdir -p $(SRC_DIR)/common/config
	@mkdir -p $(SRC_DIR)/common/security
	@mkdir -p $(SRC_DIR)/common/database
	@mkdir -p $(INCLUDE_DIR)/common
	@mkdir -p $(INCLUDE_DIR)/atm
	@mkdir -p $(INCLUDE_DIR)/admin
	@mkdir -p $(INCLUDE_DIR)/upi
	@mkdir -p $(INCLUDE_DIR)/core
	@mkdir -p $(FRONTEND_CLI_DIR)/src
	@mkdir -p $(FRONTEND_CLI_DIR)/include

# Build the ATM executable
$(ATM_EXEC): $(ATM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ATM_OBJS) $(LDFLAGS)

# Build the Admin executable
$(ADMIN_EXEC): $(ADMIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ADMIN_OBJS) $(LDFLAGS)

# Build the UPI executable
$(UPI_EXEC): $(UPI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UPI_OBJS) $(LDFLAGS)

# Build the CLI Frontend executable
$(CLI_EXEC): $(CLI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLI_OBJS) $(LDFLAGS)

# Setup database
setup_db:
	@echo "Setting up database structure..."
	@mysql -u$(DB_USER) -p$(DB_PASS) < setup/database_schema.sql
	@echo "Database setup completed."

# Clean up
clean:
	rm -f $(ATM_OBJS) $(ADMIN_OBJS) $(UPI_OBJS) $(CLI_OBJS) $(ATM_EXEC) $(ADMIN_EXEC) $(UPI_EXEC) $(CLI_EXEC)

# Dependency rule
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Web setup
web_setup:
	@echo "Setting up web environment..."
	@mkdir -p frontend/web/assets/css
	@mkdir -p frontend/web/assets/js
	@mkdir -p frontend/web/assets/images
	@mkdir -p frontend/web/pages
	@mkdir -p frontend/web/templates
	@echo "Web setup completed."

# Flutter setup
flutter_setup:
	@echo "Setting up Flutter environment..."
	@mkdir -p frontend/mobile/flutter_app
	@cd frontend/mobile/flutter_app && flutter create .
	@echo "Flutter setup completed."

# Full setup
full_setup: create_dirs web_setup flutter_setup
	@echo "Full setup completed."

.PHONY: all clean setup_db create_dirs web_setup flutter_setup full_setup
