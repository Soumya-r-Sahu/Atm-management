# Core Banking System (CBS) Logger Documentation

## Overview

The Core Banking System (CBS) logger provides comprehensive logging functionality for banking operations, with special focus on security, audit trails, and transaction logging. This system meets the regulatory requirements for financial transaction systems.

## Features

- **Multi-Level Logging**: Supports various log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL, etc.)
- **Category-Based Logging**: Organizes logs by category (APPLICATION, TRANSACTION, SECURITY, etc.)
- **Transaction Logging**: Detailed logs of all financial transactions with before/after states
- **Security Event Logging**: Records security events with appropriate severity levels
- **Audit Trail**: Comprehensive audit logging for regulatory compliance
- **PII Data Masking**: Protects sensitive information like card numbers
- **Windows Compatibility**: Simplified implementation for Windows environments

## Directory Structure

```
atm-project/
├── backend/c_backend/src/common/utils/
│   ├── cbs_logger.c
│   └── dao_audit_logger.c
├── include/common/utils/
│   ├── cbs_logger.h
│   └── dao_audit_logger.h
├── tests/
│   ├── simple_cbs_logger.h      # Simplified version for Windows
│   ├── simple_cbs_logger.c      # Simplified version for Windows
│   ├── simple_dao_audit_logger.h # Simplified version for Windows
│   ├── simple_dao_audit_logger.c # Simplified version for Windows
│   ├── simple_cbs_test.c        # Basic test program
│   ├── simple_banking_test.c    # Banking operations test
│   ├── cbs_banking_demo.c       # Comprehensive CBS demo program
│   └── test_cbs_banking.c       # Original CBS test (not Windows compatible)
└── bin/logs/                    # Log output directory
    ├── application.log
    ├── transactions.log
    ├── security.log
    ├── audit.log
    └── error.log
```

## Building the CBS Tests

### Using the Build Scripts

```bash
# Build all tests
build_all_tests.bat

# Build just the simple CBS logger test
build_simple_cbs_test.bat

# Build just the CBS banking demo
build_cbs_demo.bat
```

### Manual Build

```bash
# Build simple CBS test
gcc -o bin\simple_cbs_test.exe tests\simple_cbs_test.c tests\simple_cbs_logger.c -I. -Itests -Wall -O2

# Build simple banking test
gcc -o bin\simple_banking_test.exe tests\simple_banking_test.c -I. -Itests -Wall -O2

# Build CBS banking demo
gcc -o bin\cbs_banking_demo.exe tests\cbs_banking_demo.c tests\simple_cbs_logger.c tests\simple_dao_audit_logger.c -I. -Itests -Wall -O2
```

# CBS Banking Demo

The `cbs_banking_demo.exe` program demonstrates a complete implementation of the CBS logging system for banking operations. It includes:

1. **Account Operations**: Creation, deposits, withdrawals, and transfers
2. **Transaction Logging**: Detailed transaction logs with before/after states
3. **Security Events**: Login attempts, card operations, and security alerts
4. **PII Data Protection**: Automatic masking of sensitive information
5. **Audit Trail**: Complete audit trail for all operations

Example output from the demo:
```
CBS Banking System Logger Test
==============================

Initializing CBS Banking System Logger...
CBS Banking System initialized successfully.

Testing banking operations...
Created account ACC60100001 for customer CUST001 with balance $1000.00
Created account ACC60100002 for customer CUST002 with balance $500.00
Deposited $250.00 to account ACC60100001 (New balance: $1250.00)
Withdrew $100.00 from account ACC60100001 (New balance: $1150.00)
Transferred $200.00 from account ACC60100001 to account ACC60100002
Source account balance: $950.00
Destination account balance: $700.00

Testing insufficient funds withdrawal...
Withdrawal failed as expected due to insufficient funds

Testing invalid account transfer...
Transfer failed as expected due to invalid source account

Banking operations test completed.

Testing security logging...
Logged successful login event
Logged failed login attempt
Logged multiple failed login attempts (high severity)
Logged card block operation

Security logging test completed.

Testing PII data masking...
Logged transaction with Visa card (should be masked)
Logged transaction with Mastercard (should be masked)
Logged card operation with PII data (should be masked)

PII data masking test completed.

Shutting down CBS Banking System Logger...
CBS Banking System shut down successfully.

CBS Banking System Logger Test completed successfully.
Check the logs folder for generated log files.
```

## Running the Tests

```bash
# Run simple CBS logger test
bin\simple_cbs_test.exe

# Run simple banking test
bin\simple_banking_test.exe

# Run comprehensive CBS banking demo
bin\cbs_banking_demo.exe
```

## Log File Locations

All log files are created in the `bin\logs` directory:

- `application.log`: General application events
- `transactions.log`: Financial transaction details
- `security.log`: Security-related events
- `audit.log`: Audit trail for compliance
- `errors.log`: Error messages and stack traces

## CBS Log Levels

```c
typedef enum {
    CBS_LOG_LEVEL_DEBUG,    // Detailed debug information
    CBS_LOG_LEVEL_INFO,     // Informational messages
    CBS_LOG_LEVEL_WARNING,  // Warning conditions
    CBS_LOG_LEVEL_ERROR,    // Error conditions
    CBS_LOG_LEVEL_CRITICAL, // Critical conditions
    CBS_LOG_LEVEL_SECURITY, // Security-related logs
    CBS_LOG_LEVEL_ALERT,    // Action must be taken immediately
    CBS_LOG_LEVEL_EMERGENCY // System is unusable
} CBSLogLevel;
```

## Log Categories

```c
typedef enum {
    LOG_CATEGORY_APPLICATION, // Application events, startup, shutdown
    LOG_CATEGORY_TRANSACTION, // Financial transactions (deposit, withdrawal, etc.)
    LOG_CATEGORY_SECURITY,    // Security events (login, logout, auth failures)
    LOG_CATEGORY_DATABASE,    // Database operations
    LOG_CATEGORY_AUDIT,       // Audit trail entries (important for compliance)
    LOG_CATEGORY_SYSTEM,      // System-level events
    LOG_CATEGORY_ERROR        // Error category
} LogCategory;
```

## Basic Usage

```c
// Initialize the logger
cbs_initializeLogger("logs");

// Set the log level
cbs_setLogLevel(CBS_LOG_LEVEL_DEBUG);

// Write a simple log
cbs_writeLog(LOG_CATEGORY_APPLICATION, CBS_LOG_LEVEL_INFO, "Application started");

// Write a transaction log
cbs_writeTransactionLog(
    "TX123456",         // Transaction ID
    "USER001",          // User ID
    "1234567890123456", // Card number (will be masked)
    "ACCT001",          // Account ID
    "DEPOSIT",          // Transaction type
    100.00,             // Amount
    500.00,             // Previous balance
    600.00,             // New balance
    "SUCCESS",          // Status
    "ATM deposit"       // Details
);

// Clean up
cbs_shutdownLogger();
```

## Windows Compatibility Notes

The original implementation uses POSIX threading (pthread.h) which is not directly available on Windows. The simplified versions (`simple_cbs_logger.c` and `simple_dao_audit_logger.c`) have been created to work on Windows systems without threading dependencies.

Key differences in the Windows-compatible version:
- No pthread mutex for thread safety
- Uses Windows-specific directory creation commands
- Simplified log rotation and file handling

## Integration with DAO Audit Logger

The DAO Audit Logger provides specialized logging functions for database operations and user authentication:

```c
// Initialize DAO audit logger
init_dao_audit_logger("logs", 7);

// Record a transaction audit
record_transaction_audit(
    "USER001",          // User ID
    1234567890123456,   // Card number (will be masked)
    "DEPOSIT",          // Transaction type
    100.00,             // Amount
    500.00,             // Old balance
    600.00,             // New balance
    true                // Success flag
);

// Record authentication audit
record_auth_audit(
    "USER001",          // User ID
    "PIN",              // Authentication type
    true,               // Success flag
    "192.168.1.100",    // IP address
    "ATM Terminal #123" // Device info
);

// Clean up
close_dao_audit_logger();
```
