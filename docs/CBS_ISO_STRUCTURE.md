# Core Banking System ISO-Standard File Structure

This document explains the ISO-standard file structure for the Core Banking System project, which follows financial industry best practices and ISO standards like ISO 8583 for financial transaction messaging.

## Why ISO-Standard Structure?

The ISO-standard directory structure provides several benefits:

1. **Industry Compliance**: Follows financial sector best practices
2. **Enhanced Maintainability**: Clearly separated components with defined responsibilities
3. **Improved Collaboration**: Standardized locations make it easier for teams to work together
4. **Scalability**: Structure accommodates growth and new features
5. **Security**: Clear separation of sensitive components

## Directory Structure

```
/CBS-Project/
├── APPLICATION/        # Main application code and entry points
│   ├── main.c          # Entry point; initializes system
│   ├── config.c        # Reads system configs (file/db)
│   ├── iso8583_parser.c  # Parses ISO 8583 financial messages
│   └── iso8583_builder.c # Builds ISO 8583 messages for outbound
│
├── BACKEND/            # Server-side and business logic components
│   ├── c_backend/      # Core banking implementation
│   │   ├── src/        # Implementation files
│   │   └── include/    # Header files
│   ├── database/       # Database access layer
│   └── web_backend/    # Web services and APIs
│
├── BIN/                # Compiled executables
├── DATA/               # Configuration and reference data
├── DOCS/               # Project documentation
├── EXAMPLES/           # Example code and usage patterns
├── FRONTEND/           # User interface components
├── INCLUDE/            # Global header files
├── LOGS/               # System logs
├── SETUP/              # Installation and configuration
├── TESTS/              # Test code and frameworks
└── BUILD_FILES/        # Build system files
```

## Key Components

### APPLICATION Directory

Contains the core application code and entry points:

| File | Purpose |
|------|---------|
| `main.c` | System initialization, main loop |
| `config.c` | Configuration loading and validation |
| `iso8583_parser.c` | ISO 8583 financial message parsing |
| `iso8583_builder.c` | ISO 8583 message creation |

### BACKEND Directory

Contains server-side code and business logic:

| File | Purpose |
|------|---------|
| `c_backend/src/account.c` | Account operations (open, close, update) |
| `c_backend/src/customer.c` | Customer profile management |
| `c_backend/src/transaction.c` | Transaction processing (debit, credit) |
| `c_backend/src/ledger.c` | General ledger operations |
| `c_backend/src/audit.c` | Audit trail logging |
| `database/dao.c` | Database abstraction layer |
| `database/mysql_connector.c` | MySQL database connectivity |
| `database/file_storage.c` | File-based storage support |
| `web_backend/rest_api.c` | REST API implementation |
| `web_backend/soap_service.c` | SOAP service implementation |

### Other Key Directories

- **BIN/**: Compiled executables (atm_system.exe, admin_system.exe)
- **DATA/**: Configuration files and reference data
- **DOCS/**: Project documentation
- **LOGS/**: System logs (transactions, errors, audit)
- **TESTS/**: Unit and integration tests

## Implementation Process

To implement this structure, follow these steps:

1. **Create the directory structure**:
   ```powershell
   .\setup\create_iso_structure.ps1
   ```
   This script creates all the necessary directories according to ISO standards.

2. **Generate migration commands**:
   ```powershell
   .\setup\generate_migration_commands.ps1
   ```
   This analyzes your existing file structure and generates commands to move files to their new locations.

3. **Execute the migration**:
   ```powershell
   .\setup\migration_commands.ps1
   ```
   This moves all files to their ISO-standard locations.

4. **Verify the migration**:
   ```powershell
   .\setup\verify_iso_structure.ps1
   ```
   This checks that all directories and essential files are in place.

5. **Update include paths**:
   After migration, you'll need to update include paths in your source files to reflect the new structure.
   For example:
   - `#include "../include/config.h"` might become `#include "../INCLUDE/config.h"`
   - `#include "backend/transaction.h"` might become `#include "../BACKEND/c_backend/include/transaction.h"`

6. **Update build scripts**:
   Modify your Makefile or other build scripts to use the new directory paths.

### Linux/Unix Users

For Linux or Unix environments, use the shell script version:
```bash
./setup/implement_iso_structure.sh
```

### Full Implementation

For a complete implementation that handles all steps automatically:
```powershell
.\setup\implement_iso_structure.ps1
```
This script will:
- Create the directory structure
- Migrate files from old to new locations
- Generate a report of the migration

## ISO Standards Alignment

This structure implements several ISO standards relevant to banking systems:

* **ISO 8583** - Financial transaction messaging format
* **ISO 20022** - Universal financial industry message scheme
* **ISO 27001** - Information security management (logging structure)
* **ISO 9564** - PIN management and security

The specific standards are implemented through:
- Message format definitions in `iso8583_fields.json`
- Standard error codes in `error_codes.h`
- Security practices in logging and audit trails
- Standard API interfaces in the web backend

## Building with the ISO Structure

### Example Makefile

Here's an example of how to update your Makefile to use the ISO-standard structure:

```makefile
# Core Banking System Makefile for ISO structure

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Directories
APPLICATION_DIR = APPLICATION
BACKEND_DIR = BACKEND
INCLUDE_DIR = INCLUDE
BIN_DIR = BIN
BUILD_DIR = BUILD_FILES

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -I$(BACKEND_DIR)/c_backend/include

# Source files
APPLICATION_SRCS = $(wildcard $(APPLICATION_DIR)/*.c)
BACKEND_SRCS = $(wildcard $(BACKEND_DIR)/c_backend/src/*.c)
ALL_SRCS = $(APPLICATION_SRCS) $(BACKEND_SRCS)

# Object files
OBJS = $(ALL_SRCS:.c=.o)

# Main target
all: $(BIN_DIR)/cbs_main

$(BIN_DIR)/cbs_main: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(APPLICATION_DIR)/*.o $(BACKEND_DIR)/c_backend/src/*.o $(BIN_DIR)/cbs_main

.PHONY: all clean
```

### Example Build Command

To build the project:

```bash
make -f BUILD_FILES/Makefile
```

This will compile all the source files and create an executable in the BIN directory.
