# CBS Database Layer Reorganization Guide

This guide explains how to reorganize the database layer to follow a more structured approach.

## Current Structure

Currently, database-related files are directly in the `/backend/database/` directory:
- `account_management_dao.c`
- `admin_operations_dao.c`
- `bill_payments.c`
- etc.

## New Structure

We'll reorganize these files into the following structure:

```
/backend/database/
├── mysql/             # MySQL-specific implementation
│   ├── include/       # MySQL DAO header files
│   └── src/           # MySQL DAO implementation files
├── file_based/        # File-based implementation
│   ├── include/       # File-based DAO header files
│   └── src/           # File-based implementation files
└── common/            # Shared database utilities
    ├── include/       # Common DAO interfaces
    └── src/           # Common DAO utilities
```

## Migration Steps

### 1. Create the new directory structure

Use the provided `reorganize_project.ps1` script to create the new directories.

### 2. Analyze and categorize existing files

Categorize each file in `/backend/database/` based on its purpose:

| File | Category | New Location |
|------|----------|--------------|
| `account_management_dao.c` | Interface | `/backend/database/common/include/` |
| `mysql_dao.c` | MySQL Implementation | `/backend/database/mysql/src/` |
| `file_based_dao.c` | File-based Implementation | `/backend/database/file_based/src/` |
| `dao_factory.c` | Common | `/backend/database/common/src/` |

### 3. Create header files for all DAO components

For each implementation file, ensure there is a corresponding header file:

- `mysql_dao.h` in `/backend/database/mysql/include/`
- `file_based_dao.h` in `/backend/database/file_based/include/`
- `dao_factory.h` in `/backend/database/common/include/`

### 4. Update include paths in source files

After moving the files, update all include paths to reflect the new structure:

```c
// Old
#include "mysql_dao.h"

// New
#include "database/mysql/include/mysql_dao.h"
```

### 5. Update the build scripts

Modify the Makefile to include the new directories:

```makefile
# Add all database subdirectories to include path
DATABASE_INCLUDES = -I$(SRC_DIR)/backend/database/mysql/include \
                   -I$(SRC_DIR)/backend/database/file_based/include \
                   -I$(SRC_DIR)/backend/database/common/include

# Define source files by implementation
MYSQL_DAO_SRC = $(wildcard $(SRC_DIR)/backend/database/mysql/src/*.c)
FILE_DAO_SRC = $(wildcard $(SRC_DIR)/backend/database/file_based/src/*.c)
COMMON_DAO_SRC = $(wildcard $(SRC_DIR)/backend/database/common/src/*.c)

# Create object files
MYSQL_DAO_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(MYSQL_DAO_SRC))
FILE_DAO_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(FILE_DAO_SRC))
COMMON_DAO_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(COMMON_DAO_SRC))
```

### 6. Step-by-Step Migration Process

To minimize risks:

1. **Create the directories** using the reorganization script
2. **Copy (don't move)** files to their new locations
3. **Update include paths** in the copied files
4. **Update the build script** to use the new locations
5. **Test the build** to make sure everything compiles
6. **Run tests** to ensure functionality is preserved
7. **Remove the old files** once everything is working

### 7. Testing After Migration

After completing the migration:
- Run unit tests for all DAO functions
- Test both MySQL and file-based implementations
- Verify that the DAO factory correctly creates the right implementation
- Check for any regression in database operations

## Benefits of Reorganization

This new structure provides:
- Clear separation between different database implementations
- Better organization of interface and implementation code
- Easier addition of new database backends in the future
- Improved maintainability and readability
