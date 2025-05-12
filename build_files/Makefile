# Core Banking System Build File
# Makefile for Windows and Linux environments

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

# MySQL and libcurl flags - adjust paths as necessary
# For Windows with MinGW:
ifeq ($(OS),Windows_NT)
    MYSQL_CFLAGS = -I"C:/Program Files/MySQL/MySQL Server 8.0/include"
    MYSQL_LIBS = -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysqlclient
    CURL_CFLAGS = -I"C:/Program Files/curl/include"
    CURL_LIBS = -L"C:/Program Files/curl/lib" -lcurl
    TARGET_EXT = .exe
    RM = del /Q
    MKDIR = if not exist "$1" mkdir "$1"
    SYS_LIBS = -lws2_32
else
    # For Linux:
    MYSQL_CFLAGS = $(shell mysql_config --cflags)
    MYSQL_LIBS = $(shell mysql_config --libs)
    CURL_CFLAGS = $(shell curl-config --cflags)
    CURL_LIBS = $(shell curl-config --libs)
    TARGET_EXT =
    RM = rm -f
    MKDIR = mkdir -p $1
    SYS_LIBS =
endif

# Directories
SRC_DIR = APPLICATION
BACKEND_DIR = BACKEND
INCLUDE_DIR = INCLUDE
BIN_DIR = BIN
OBJ_DIR = BUILD_FILES/obj
TEST_DIR = TESTS

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -I$(BACKEND_DIR)/c_backend/include

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(BACKEND_DIR)/c_backend/src/*.c) \
       $(wildcard $(BACKEND_DIR)/database/*.c) \
       $(wildcard $(BACKEND_DIR)/web_backend/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)) \
       $(patsubst $(BACKEND_DIR)/c_backend/src/%.c,$(OBJ_DIR)/backend_%.o,$(wildcard $(BACKEND_DIR)/c_backend/src/*.c)) \
       $(patsubst $(BACKEND_DIR)/database/%.c,$(OBJ_DIR)/db_%.o,$(wildcard $(BACKEND_DIR)/database/*.c)) \
       $(patsubst $(BACKEND_DIR)/web_backend/%.c,$(OBJ_DIR)/web_%.o,$(wildcard $(BACKEND_DIR)/web_backend/*.c))

# Main targets
MAIN_TARGET = $(BIN_DIR)/cbs_server$(TARGET_EXT)
API_TARGET = $(BIN_DIR)/api_server$(TARGET_EXT)
ATM_TARGET = $(BIN_DIR)/atm_client$(TARGET_EXT)
ADMIN_TARGET = $(BIN_DIR)/admin_dashboard$(TARGET_EXT)

# Test targets
TEST_UNIT_TARGET = $(BIN_DIR)/unit_tests$(TARGET_EXT)
TEST_INTEGRATION_TARGET = $(BIN_DIR)/integration_tests$(TARGET_EXT)
TEST_ISO_TARGET = $(BIN_DIR)/test_iso8583$(TARGET_EXT)
TEST_TXN_TARGET = $(BIN_DIR)/test_transaction$(TARGET_EXT)
TEST_API_TARGET = $(BIN_DIR)/test_api_request$(TARGET_EXT)

# Main build rules
all: create_dirs $(MAIN_TARGET) $(API_TARGET) $(ATM_TARGET) $(ADMIN_TARGET) tests

# Create necessary directories
create_dirs:
	@echo Creating directories...
ifeq ($(OS),Windows_NT)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
else
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
endif

# Main server application
$(MAIN_TARGET): $(OBJ_DIR)/main.o $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building main server...
	$(CC) $(CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# API server application
$(API_TARGET): $(OBJS)
	@echo Building API server...
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/web_rest_api.o $(OBJ_DIR)/web_api_utils.o $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# ATM client application
$(ATM_TARGET): $(OBJ_DIR)/frontend_atm.o $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building ATM client...
	$(CC) $(CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# Admin dashboard application
$(ADMIN_TARGET): $(OBJ_DIR)/frontend_dashboard.o $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building admin dashboard...
	$(CC) $(CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# Compile APPLICATION source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -c $< -o $@

# Compile BACKEND source files
$(OBJ_DIR)/backend_%.o: $(BACKEND_DIR)/c_backend/src/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -c $< -o $@

# Compile DATABASE source files
$(OBJ_DIR)/db_%.o: $(BACKEND_DIR)/database/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -c $< -o $@

# Compile WEB_BACKEND source files
$(OBJ_DIR)/web_%.o: $(BACKEND_DIR)/web_backend/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -c $< -o $@

# Compile FRONTEND source files
$(OBJ_DIR)/frontend_%.o: FRONTEND/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -c $< -o $@

# Build test targets
tests: $(TEST_UNIT_TARGET) $(TEST_INTEGRATION_TARGET) $(TEST_ISO_TARGET) $(TEST_TXN_TARGET) $(TEST_API_TARGET)

# Unit tests
$(TEST_UNIT_TARGET): $(TEST_DIR)/unit_tests.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building unit tests...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# Integration tests
$(TEST_INTEGRATION_TARGET): $(TEST_DIR)/integration_tests.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building integration tests...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# ISO 8583 example test
$(TEST_ISO_TARGET): EXAMPLES/test_iso8583.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building ISO 8583 test...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# Transaction example test
$(TEST_TXN_TARGET): EXAMPLES/test_transaction.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building transaction test...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# API request example test
$(TEST_API_TARGET): EXAMPLES/test_api_request.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
	@echo Building API request test...
	$(CC) $(CFLAGS) $(INCLUDES) $(MYSQL_CFLAGS) $(CURL_CFLAGS) -o $@ $^ $(MYSQL_LIBS) $(CURL_LIBS) $(LDFLAGS) $(SYS_LIBS)

# Clean build files
clean:
	@echo Cleaning up...
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJ_DIR)" rd /S /Q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)\*.exe" del /Q "$(BIN_DIR)\*.exe"
else
	@rm -rf $(OBJ_DIR)
	@rm -f $(BIN_DIR)/*
endif

# Print help information
help:
	@echo "Core Banking System Build System"
	@echo "================================"
	@echo "Targets:"
	@echo "  all              - Build all targets (default)"
	@echo "  $(MAIN_TARGET)   - Build main server application"
	@echo "  $(API_TARGET)    - Build API server application"
	@echo "  $(ATM_TARGET)    - Build ATM client application"
	@echo "  $(ADMIN_TARGET)  - Build admin dashboard application"
	@echo "  tests            - Build all test applications"
	@echo "  clean            - Remove all build files"
	@echo "  help             - Display this help message"

.PHONY: all create_dirs tests clean help
