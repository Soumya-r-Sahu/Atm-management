# ATM Project Master Makefile
# Reorganized structure with clean backend/frontend separation

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Check if MySQL is available and add appropriate flags
MYSQL_CONFIG := $(shell where mysql_config 2>NUL)

# Check for MySQL in XAMPP
XAMPP_MYSQL := $(shell if exist "C:\xampp\mysql\bin\mysql.exe" (echo true) else (echo false))

# Allow overriding with NO_MYSQL flag
ifeq ($(findstring -DNO_MYSQL,$(CFLAGS)),)
    # NO_MYSQL not specified, check for MySQL
    ifdef MYSQL_CONFIG
        MYSQL_CFLAGS := $(shell mysql_config --cflags)
        MYSQL_LDFLAGS := $(shell mysql_config --libs)
        CFLAGS += $(MYSQL_CFLAGS) -DHAVE_MYSQL
        LDFLAGS += $(MYSQL_LDFLAGS)
    else ifeq ($(XAMPP_MYSQL),true)
        # Use XAMPP MySQL
        CFLAGS += -I"C:/xampp/mysql/include" -DHAVE_MYSQL
        LDFLAGS += -L"C:/xampp/mysql/lib" -lmysql
    else
        # MySQL not found, use stub implementation
        CFLAGS += -DNO_MYSQL
    endif
endif

# Project directories
PROJECT_ROOT = .
INCLUDE_DIR = $(PROJECT_ROOT)/include
BACKEND_DIR = $(PROJECT_ROOT)/backend
FRONTEND_DIR = $(PROJECT_ROOT)/frontend
C_BACKEND_DIR = $(BACKEND_DIR)/c_backend
C_BACKEND_SRC = $(C_BACKEND_DIR)/src
C_BACKEND_INCLUDE = $(C_BACKEND_DIR)/include
FRONTEND_CLI_DIR = $(FRONTEND_DIR)/cli
BIN_DIR = $(PROJECT_ROOT)/bin
DATA_DIR = $(PROJECT_ROOT)/data
LOGS_DIR = $(PROJECT_ROOT)/logs
TESTS_DIR = $(PROJECT_ROOT)/tests

# Include directories
INCLUDES = -I$(INCLUDE_DIR) \
           -I$(INCLUDE_DIR)/admin \
           -I$(INCLUDE_DIR)/atm \
           -I$(INCLUDE_DIR)/common \
           -I$(INCLUDE_DIR)/netbanking \
           -I$(INCLUDE_DIR)/upi_transaction

# Source files for main components
COMMON_SRCS = $(wildcard $(C_BACKEND_SRC)/common/*/*.c)
ATM_SRCS = $(wildcard $(C_BACKEND_SRC)/atm/*/*.c) $(wildcard $(C_BACKEND_SRC)/atm/*.c)
ADMIN_SRCS = $(wildcard $(C_BACKEND_SRC)/admin/*/*.c) $(wildcard $(C_BACKEND_SRC)/admin/*.c)
DATABASE_SRCS = $(wildcard $(BACKEND_DIR)/database/*.c)
CLI_SRCS = $(wildcard $(FRONTEND_CLI_DIR)/*.c) $(wildcard $(FRONTEND_CLI_DIR)/src/*.c)
TEST_SRCS = $(wildcard $(TESTS_DIR)/*.c)

# Mock implementation for tests
MOCK_SRCS = $(C_BACKEND_SRC)/atm/transaction/atm_mocks_complete.c

# Generate object file names
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
ATM_OBJS = $(ATM_SRCS:.c=.o)
ADMIN_OBJS = $(ADMIN_SRCS:.c=.o)
DATABASE_OBJS = $(DATABASE_SRCS:.c=.o)
CLI_OBJS = $(CLI_SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)
MOCK_OBJS = $(MOCK_SRCS:.c=.o)

# Final executable names
# Add .exe extension for Windows
ATM_SYSTEM = $(BIN_DIR)/atm_system.exe
ADMIN_SYSTEM = $(BIN_DIR)/admin_system.exe
CLI_FRONTEND = $(BIN_DIR)/atm_cli.exe
CBS_TEST = $(BIN_DIR)/test_cbs_functionality.exe

# Default target: build all executables
all: directories $(ATM_SYSTEM) $(ADMIN_SYSTEM) $(CLI_FRONTEND) tests

# Build test targets
BUILD_TEST = $(BIN_DIR)/test_build_setup

# Simple build test target
BUILD_TEST = $(BIN_DIR)/test_build_setup.exe

build_test: directories $(BUILD_TEST)
	@echo Build test target created successfully.
	@echo Run with: $(BUILD_TEST)

$(BUILD_TEST): tests/test_build_setup.c
	@echo Building build test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo Build test created successfully.

# Create necessary directories
directories:
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	@if not exist $(DATA_DIR) mkdir $(DATA_DIR)
	@if not exist $(LOGS_DIR) mkdir $(LOGS_DIR)
	@if not exist $(DATA_DIR)\temp mkdir $(DATA_DIR)\temp
	@if not exist $(LOGS_DIR) mkdir $(LOGS_DIR)

# Clean build files
clean:
	@echo Cleaning build files...
	@del /S /Q *.o *.exe
	@del /Q $(BIN_DIR)\*.exe

# MySQL test target
MYSQL_TEST = $(BIN_DIR)/test_mysql.exe

mysql_test: directories $(MYSQL_TEST)
	@echo MySQL test target created successfully.
	@echo Run with: $(MYSQL_TEST)

$(MYSQL_TEST): tests/test_mysql.c
	@echo Building MySQL test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo MySQL test built successfully.

# Simple MySQL test target
MYSQL_SIMPLE_TEST = $(BIN_DIR)/test_mysql_simple.exe

mysql_simple_test: directories $(MYSQL_SIMPLE_TEST)
	@echo Simple MySQL test target created successfully.
	@echo Run with: $(MYSQL_SIMPLE_TEST)

$(MYSQL_SIMPLE_TEST): tests/test_mysql_simple.c
	@echo Building Simple MySQL test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo Simple MySQL test built successfully.

# ATM Database test target
ATM_DB_TEST = $(BIN_DIR)/test_atm_db.exe

atm_db_test: directories $(ATM_DB_TEST)
	@echo ATM Database test target created successfully.
	@echo Run with: $(ATM_DB_TEST)

$(ATM_DB_TEST): tests/test_atm_db.c $(COMMON_OBJS) $(DATABASE_OBJS)
	@echo Building ATM Database test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
	@echo ATM Database test built successfully.

# Database interface test target
DB_INTERFACE_TEST = $(BIN_DIR)/test_db_interface.exe

db_interface_test: directories $(DB_INTERFACE_TEST)
	@echo Database interface test target created successfully.
	@echo Run with: $(DB_INTERFACE_TEST)

$(DB_INTERFACE_TEST): tests/test_db_interface.c
	@echo Building Database interface test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo Database interface test built successfully.

# DAO implementation test target
DAO_TEST = $(BIN_DIR)/test_dao_implementation.exe

dao_test: directories $(DAO_TEST)
	@echo DAO implementation test target created successfully.
	@echo Run with: $(DAO_TEST)

$(DAO_TEST): tests/test_dao_implementation.c $(DATABASE_OBJS)
	@echo Building DAO implementation test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
	@echo DAO implementation test built successfully.

# SQL test harness target
SQL_HARNESS_TEST = $(BIN_DIR)/test_sql_harness.exe

sql_harness_test: directories $(SQL_HARNESS_TEST)
	@echo SQL test harness created successfully.
	@echo Run with: $(SQL_HARNESS_TEST)

$(SQL_HARNESS_TEST): tests/test_sql_harness_fixed.c
	@echo Building SQL test harness...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) -lm
	@echo SQL test harness built successfully.

# Basic SQL test target
SQL_BASIC_TEST = $(BIN_DIR)/test_sql_basic.exe

sql_basic_test: directories $(SQL_BASIC_TEST)
	@echo Basic SQL test created successfully.
	@echo Run with: $(SQL_BASIC_TEST)

$(SQL_BASIC_TEST): tests/test_sql_basic.c
	@echo Building basic SQL test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)
	@echo Basic SQL test built successfully.

# Simple SQL test target
SQL_SIMPLE_TEST = $(BIN_DIR)/test_sql_simple.exe

sql_simple_test: directories $(SQL_SIMPLE_TEST)
	@echo Simple SQL test created successfully.
	@echo Run with: $(SQL_SIMPLE_TEST)

$(SQL_SIMPLE_TEST): tests/test_sql_simple.c $(BACKEND_DIR)/c_backend/src/common/database/mysql_stub.c
	@echo Building simple SQL test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -DNO_MYSQL $(INCLUDES) -o $@ $^ $(LDFLAGS) -lm
	@echo Simple SQL test built successfully.

# Self-contained MySQL stub test target
MYSQL_STUB_TEST = $(BIN_DIR)/test_mysql_stub.exe

mysql_stub_test: directories $(MYSQL_STUB_TEST)
	@echo MySQL stub test created successfully.
	@echo Run with: $(MYSQL_STUB_TEST)

$(MYSQL_STUB_TEST): tests/test_mysql_stub_test.c
	@echo Building MySQL stub test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< -lm
	@echo MySQL stub test built successfully.

# MySQL verification test target
VERIFY_MYSQL = $(BIN_DIR)/verify_mysql.exe

verify_mysql: directories $(VERIFY_MYSQL)
	@echo MySQL verification test created successfully.
	@echo Run with: $(VERIFY_MYSQL)

$(VERIFY_MYSQL): tests/verify_mysql.c
	@echo Building MySQL verification test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) -lm
	@echo MySQL verification test built successfully.

# Run MySQL verification with real MySQL support
verify-mysql-real: verify_mysql
	@echo Running MySQL verification with real MySQL support...
	@if exist $(VERIFY_MYSQL) ( \
		echo. && \
		echo Running MySQL verification... && \
		$(VERIFY_MYSQL) \
	) else ( \
		echo. && \
		echo Error: MySQL verification executable not found. \
	)

# Run MySQL verification with MySQL stub
verify-mysql-stub: 
	@echo Running MySQL verification with stub implementation...
	@mingw32-make verify_mysql CFLAGS+=-DNO_MYSQL
	@if exist $(VERIFY_MYSQL) ( \
		echo. && \
		echo Running MySQL verification with stub... && \
		$(VERIFY_MYSQL) \
	) else ( \
		echo. && \
		echo Error: MySQL verification executable not found. \
	)

# Build the ATM system executable
$(ATM_SYSTEM): $(COMMON_OBJS) $(ATM_OBJS) $(DATABASE_OBJS)
	@echo Building ATM System...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ATM System built successfully.

# Build the Admin system executable
$(ADMIN_SYSTEM): $(COMMON_OBJS) $(ADMIN_OBJS) $(DATABASE_OBJS)
	@echo Building Admin System...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Admin System built successfully.

# Build the CLI Frontend executable
$(CLI_FRONTEND): $(CLI_OBJS) $(COMMON_OBJS) 
	@echo Building CLI Frontend...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo CLI Frontend built successfully.

# Build and run tests
tests: $(CBS_TEST)
	@echo All tests built successfully.

# Build CBS functionality tests
$(CBS_TEST): $(TESTS_DIR)/test_cbs_functionality.c $(MOCK_OBJS) $(COMMON_OBJS) 
	@echo Building CBS Functionality Test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
	@echo CBS Functionality Test built successfully.

# Command for running tests
run_tests: tests
	@echo Running CBS Functionality Test...
	$(CBS_TEST) 5555123412341234
	@echo Tests completed.

# Compile individual source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Verify code organization
verify_organization:
	@echo Verifying code organization...
	@powershell -Command "$$backendFiles = Get-ChildItem -Path '$(FRONTEND_DIR)' -Recurse -Include *.c,*.h | Select-String -Pattern '#include.*backend' -List; if ($$backendFiles) { Write-Output 'WARNING: Backend includes found in frontend:'; $$backendFiles | ForEach-Object { Write-Output $$_.Path } } else { Write-Output 'No backend includes found in frontend files. Good!' }"
	@powershell -Command "$$frontendFiles = Get-ChildItem -Path '$(BACKEND_DIR)' -Recurse -Include *.c,*.h | Select-String -Pattern '#include.*frontend' -List; if ($$frontendFiles) { Write-Output 'WARNING: Frontend includes found in backend:'; $$frontendFiles | ForEach-Object { Write-Output $$_.Path } } else { Write-Output 'No frontend includes found in backend files. Good!' }"
	@echo Code organization verification completed.

# Prepare for no MySQL build by creating modified source files
prepare-no-mysql:
	@echo Creating build directory with modified sources for MySQL-less build...
	@if not exist build_tmp mkdir build_tmp
	@echo Copying source files...
	@xcopy /E /I /Y backend build_tmp\backend
	@xcopy /E /I /Y frontend build_tmp\frontend
	@xcopy /E /I /Y include build_tmp\include
	@echo Modifying MySQL includes...
	@powershell -Command "Get-ChildItem -Path 'build_tmp' -Recurse -Include *.c,*.h | ForEach-Object { $$content = Get-Content $$_.FullName; if ($$content -match '#include <mysql/mysql.h>') { Write-Host \"Fixing MySQL include in $$_\"; $$content -replace '#include <mysql/mysql.h>', '/* MySQL include removed */' | Set-Content $$_.FullName } }"
	@echo Build directory prepared for MySQL-less build.

# DAO implementation test target
DAO_TEST = $(BIN_DIR)/test_dao_implementation.exe

dao_test: directories $(DAO_TEST)
	@echo DAO implementation test target created successfully.
	@echo Run with: $(DAO_TEST)

$(DAO_TEST): tests/test_dao_implementation.c $(BACKEND_DIR)/database/dao_factory.c $(BACKEND_DIR)/database/file_based_dao.c $(BACKEND_DIR)/database/mysql_dao.c $(COMMON_OBJS)
	@echo Building DAO implementation test...
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) -lm
	@echo DAO implementation test built successfully.

# Help target
help:
	@echo ATM Project Makefile
	@echo --------------------
	@echo Available targets:
	@echo   all             - Build all executables (default)
	@echo   clean           - Remove all build files
	@echo   tests           - Build all test executables
	@echo   run_tests       - Build and run all tests
	@echo   verify_organization - Check for proper code separation
	@echo   prepare-no-mysql - Create build dir with modified sources for MySQL-less build
	@echo   verify-mysql-real - Build and run MySQL verification with real MySQL
	@echo   verify-mysql-stub - Build and run MySQL verification with stub implementation
	@echo   mysql_test, mysql_simple_test - Build MySQL tests
	@echo   atm_db_test, db_interface_test - Build database tests
	@echo   sql_harness_test, sql_basic_test, sql_simple_test - Build SQL tests
	@echo   mysql_stub_test - Build MySQL stub test
	@echo   dao_test        - Build and test the DAO implementation
	@echo   help            - Display this help message

.PHONY: all clean directories tests run_tests verify_organization prepare-no-mysql help

COMMON_SRCS += backend/c_backend/src/common/database/mysql_stub.c

