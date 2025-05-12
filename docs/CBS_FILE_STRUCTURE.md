# CBS Project File Structure Documentation

This document describes the detailed file structure of the Core Banking System project, explaining the purpose and contents of each folder and file.

## Root Directory

```
/CBS-Project/
```

The root directory contains the main configuration files, build scripts, and subdirectories that organize the project's components.

| File/Folder | Description |
|-------------|-------------|
| `README.md` | Main project documentation and quick start guide |
| `Makefile` | Primary build script for compiling the entire project |
| `BUG_TRACKING.md` | List of known bugs, issues and their status |
| `setup_mysql.bat` | Windows script for setting up the MySQL database |
| `setup_mysql.sh` | Linux/Unix script for setting up the MySQL database |

## Application Directory

```
/application/
├── config/
```

Contains the main application code and configuration settings.

| File/Folder | Description |
|-------------|-------------|
| `config/` | Application configuration files and settings |

## Backend Directory

```
/backend/
├── c_backend/
├── database/
└── web_backend/
```

Contains all server-side code, database access layers, and core business logic.

### C Backend

```
/backend/c_backend/
```

Core banking system logic implemented in C.

| Component | Description |
|-----------|-------------|
| Account Management | Functions for account creation, updates, and closures |
| Customer Management | Customer profile handling and data management |
| Transaction Processing | Core transaction logic (deposits, withdrawals, transfers) |
| Business Logic | Banking rules, validations, and calculations |

### Database Layer

```
/backend/database/
```

Database access objects, connection management, and data persistence logic.

| Component | Description |
|-----------|-------------|
| Database Access Objects | Implementation of data access patterns |
| Connection Management | Database connection pooling and management |
| Query Builders | SQL query construction and execution |
| Transaction Management | Database transaction control |

### Web Backend

```
/backend/web_backend/
```

Web service implementations, APIs, and network-facing components.

| Component | Description |
|-----------|-------------|
| REST APIs | REST endpoint implementations |
| SOAP Services | SOAP service handlers |
| API Authentication | Request authentication and authorization |
| Response Formatting | Standardized API response handling |

## Binary Directory

```
/bin/
├── admin_system
├── atm_system
├── upi_system
└── [test executables]
```

Contains compiled executables and binaries.

| File | Description |
|------|-------------|
| `admin_system` | Administrative interface executable |
| `atm_system` | ATM interface system executable |
| `upi_system` | UPI transaction processing executable |
| `*_test.exe` | Various test executables |
| `logs/` | Runtime logs from binary executions |

## Build Directory

```
/build/
└── makefiles/
```

Build configuration files and specialized makefiles.

| File/Folder | Description |
|-------------|-------------|
| `makefiles/` | Module-specific makefiles for component building |

## Data Directory

```
/data/
├── accounting.txt
├── admin_credentials.txt
├── atm_config.txt
├── atm_data.txt
├── card.txt
├── customer.txt
├── languages.txt
├── system_config.txt
├── virtual_wallet.txt
└── temp/
```

Configuration files, reference data, and temporary storage.

| File/Folder | Description |
|-------------|-------------|
| `accounting.txt` | Accounting rules and configuration |
| `admin_credentials.txt` | Administrator access credentials |
| `atm_config.txt` | ATM machine configuration settings |
| `atm_data.txt` | ATM transaction and status data |
| `card.txt` | Card information and configuration |
| `customer.txt` | Customer reference data |
| `languages.txt` | Localization and language settings |
| `system_config.txt` | Global system configuration |
| `virtual_wallet.txt` | Digital wallet configuration |
| `temp/` | Temporary data storage directory |

## Documentation Directory

```
/docs/
├── CBS_BUILD_GUIDE.md
├── CBS_DATABASE_REORGANIZATION_GUIDE.md
├── CBS_FUNCTION_REFERENCE.md
├── CBS_IMPLEMENTATION_PLAN.md
├── CBS_LOGGING_GUIDE.md
├── CBS_LOGGING_UPDATE_GUIDE.md
├── CBS_PROJECT_DOCUMENTATION.md
├── CBS_REORGANIZATION_PLAN.md
├── CBS_SYSTEM_ARCHITECTURE.md
├── CBS_TESTING_PLAN.md
├── README.md
├── REORGANIZATION_SUMMARY.md
└── archive/
```

Project documentation, guides, and reference materials.

| File/Folder | Description |
|-------------|-------------|
| `CBS_BUILD_GUIDE.md` | Instructions for building the project |
| `CBS_DATABASE_REORGANIZATION_GUIDE.md` | Guide for database structure reorganization |
| `CBS_FUNCTION_REFERENCE.md` | Reference for project functions and APIs |
| `CBS_IMPLEMENTATION_PLAN.md` | Implementation roadmap and plan |
| `CBS_LOGGING_GUIDE.md` | Guide for using the logging system |
| `CBS_LOGGING_UPDATE_GUIDE.md` | Guide for updating the logging system |
| `CBS_PROJECT_DOCUMENTATION.md` | Main project documentation |
| `CBS_REORGANIZATION_PLAN.md` | Project reorganization plan |
| `CBS_SYSTEM_ARCHITECTURE.md` | System architecture diagrams and explanations |
| `CBS_TESTING_PLAN.md` | Testing strategy and procedures |
| `README.md` | Documentation index |
| `REORGANIZATION_SUMMARY.md` | Summary of reorganization efforts |
| `archive/` | Historical and deprecated documentation |

## Examples Directory

```
/examples/
├── transaction_processor_dao.c
```

Example code and usage patterns.

| File/Folder | Description |
|-------------|-------------|
| `transaction_processor_dao.c` | Example of transaction processing with DAO pattern |

## Frontend Directory

```
/frontend/
├── cli/
```

User interface implementations.

| File/Folder | Description |
|-------------|-------------|
| `cli/` | Command-line interface components |

## Include Directory

```
/include/
├── build_config.h
├── admin/
├── atm/
├── common/
├── frontend/
├── netbanking/
├── tests/
└── upi_transaction/
```

Header files, shared declarations, and public interfaces.

| File/Folder | Description |
|-------------|-------------|
| `build_config.h` | Build configuration header |
| `admin/` | Admin module headers |
| `atm/` | ATM module headers |
| `common/` | Shared utility headers |
| `frontend/` | Frontend component headers |
| `netbanking/` | Net banking module headers |
| `tests/` | Test framework headers |
| `upi_transaction/` | UPI transaction headers |

## Logs Directory

```
/logs/
├── application.log
├── atm_transactions.log
├── audit.log
├── error.log
├── errors.log
├── info.log
├── security.log
└── transactions.log
```

System logs and operation records.

| File/Folder | Description |
|-------------|-------------|
| `application.log` | General application events |
| `atm_transactions.log` | ATM-specific transaction logs |
| `audit.log` | Security and access audit trail |
| `error.log` | Error and exception records |
| `errors.log` | Categorized error logs |
| `info.log` | Informational logs |
| `security.log` | Security events and violations |
| `transactions.log` | Financial transaction records |

## Setup Directory

```
/setup/
├── migrate_to_mysql.sh
├── migration_helper.ps1
├── reorganize_project.ps1
├── sync_db_config.bat
├── sync_db_config.sh
├── validate_reorganization.ps1
├── php/
└── sql/
```

Installation, configuration, and setup scripts.

| File/Folder | Description |
|-------------|-------------|
| `migrate_to_mysql.sh` | Script for migrating data to MySQL |
| `migration_helper.ps1` | PowerShell helper for project migration |
| `reorganize_project.ps1` | Script for reorganizing project structure |
| `sync_db_config.bat` | Windows script to sync database configurations |
| `sync_db_config.sh` | Linux/Unix script to sync database configurations |
| `validate_reorganization.ps1` | Script to validate reorganization |
| `php/` | PHP configuration and setup scripts |
| `sql/` | SQL scripts for database setup and migration |

## Tests Directory

```
/tests/
├── basic_test.c
├── build.bat
├── cbs_banking_demo.c
├── dao_interface.h
├── init_data_files.c
├── Makefile
├── Makefile.cbs_banking
├── Makefile.cbs_logger
├── Makefile.security
├── mock_dao.c
├── simple_banking_test.c
├── simple_cbs_logger.c
├── simple_cbs_logger.h
├── simple_cbs_test.c
├── simple_dao_audit_logger.c
├── simple_dao_audit_logger.h
├── simple_dao_test.c
├── simple_logger.c
├── test_admin_operations_dao.c
├── test_atm_db.c
├── test_build_setup.c
├── test_cbs_banking.c
├── test_cbs_functionality.c
├── test_cbs_logger.c
├── test_connection_pool.c
├── test_core_banking.c
├── test_dao_implementation.c
├── test_dao_with_framework.c
├── test_db_interface.c
├── test_mysql_simple.c
├── test_mysql_stub_test.c
└── test_mysql.c
```

Test scripts, cases, and frameworks.

| File/Folder | Description |
|-------------|-------------|
| `basic_test.c` | Basic functionality tests |
| `build.bat` | Windows test build script |
| `cbs_banking_demo.c` | Banking demo test application |
| `dao_interface.h` | Data Access Object interface header |
| `init_data_files.c` | Test data initialization |
| `Makefile` | Main test makefile |
| `Makefile.*` | Component-specific test makefiles |
| `mock_dao.c` | Mock database implementation for testing |
| `simple_*` | Simplified test implementations |
| `test_*.c` | Specific component test files |

## Reorganized Structure (Proposed)

After implementing the reorganization plans, the structure will be modified as follows:

### Database Layer (New)

```
/backend/database/
├── mysql/
│   ├── include/         # MySQL DAO header files
│   └── src/             # MySQL DAO implementation files
├── file_based/
│   ├── include/         # File-based DAO header files
│   └── src/             # File-based implementation files
└── common/
    ├── include/         # Common DAO interfaces
    └── src/             # Common DAO utilities
```

### Logs Directory (New)

```
/logs/
├── transactions/        # All transaction-related logs
├── security/            # Security and authentication logs
├── errors/              # Error logs
└── audit/               # Audit trail logs
```

### Binaries Directory (New)

```
/bin/
├── production/          # Production executables
├── testing/             # Test binaries
└── debug/               # Debug builds
```

### Build Directory (New)

```
/build/
├── makefiles/           # Specific module makefiles
├── scripts/             # Build automation scripts
└── config/              # Build configuration files
```

## File Content Overview

### Core Banking Components

| Component | Functionality |
|-----------|--------------|
| **Account Management** | Account creation, modification, closure, balance management |
| **Customer Management** | Customer profiles, KYC, relationship management |
| **Transaction Processing** | Deposits, withdrawals, transfers, payments |
| **Card Management** | Debit/credit card issuance, blocking, PIN management |
| **UPI Services** | UPI transaction processing, virtual payment addressing |
| **Reporting** | Transaction reports, statements, audit records |

### Database Layer

| Component | Functionality |
|-----------|--------------|
| **DAO Implementation** | Data access patterns for different storage backends |
| **Connection Management** | Database connection pooling and lifecycle |
| **Data Persistence** | Storage and retrieval of banking data |
| **Transaction Control** | ACID properties for database transactions |

### Logging System

| Component | Functionality |
|-----------|--------------|
| **Transaction Logging** | Recording financial transaction details |
| **Security Logging** | Authentication events, access control |
| **Error Logging** | System errors and exceptions |
| **Audit Logging** | Compliance and audit trail records |

### Building and Testing

| Component | Functionality |
|-----------|--------------|
| **Makefiles** | Build processes for different components |
| **Test Scripts** | Unit, integration, and system tests |
| **Mock Objects** | Test doubles for external dependencies |
