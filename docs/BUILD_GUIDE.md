# ATM Project Build Guide

This guide provides instructions for building the ATM Management System project with different configurations.

## Prerequisites

- GCC compiler (MinGW on Windows)
- Make utility (mingw32-make on Windows)
- MySQL development libraries (optional)

## Build Options

### 1. Standard Build (with MySQL)

If you have MySQL development libraries installed:

```bash
mingw32-make all
```

This will build all components including:
- ATM System
- Admin System
- CLI Frontend
- Tests

### 2. MySQL-less Build

If you don't have MySQL installed:

```bash
mingw32-make all CFLAGS+="-DNO_MYSQL"
```

This build uses stub implementations for database operations.

### 3. Basic Test Build

To verify the build setup works:

```bash
gcc -Wall -g -DNO_MYSQL -o bin\basic_test.exe tests\basic_test.c
bin\basic_test.exe
```

### 4. Verify Code Organization

To check that code follows proper organization principles:

```bash
mingw32-make verify_organization
```

## Directory Structure

- `frontend/` - User interface implementation
  - `cli/` - Command-line interface
  - `web/` - Web interface (if applicable)

- `backend/` - Business logic and data handling
  - `c_backend/` - C implementation of backend
  - `database/` - Database access and management

- `include/` - Public header files
  - `common/` - Common utilities and shared headers
  - `atm/` - ATM-specific interfaces
  - `admin/` - Administration interfaces

- `bin/` - Compiled executables

- `data/` - Data storage

- `logs/` - Log files

## Common Issues

### MySQL Headers Not Found

If you see errors like:
```
error: mysql/mysql.h: No such file or directory
```

Use the MySQL-less build option:
```bash
mingw32-make all CFLAGS+="-DNO_MYSQL"
```

### Build Directory Issues

Make sure all required directories exist:
```bash
mkdir -p bin data logs
```

### MySQL Configuration

If MySQL is installed but not detected, check that:
1. MySQL is in your PATH
2. mysql_config is available
