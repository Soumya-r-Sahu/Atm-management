# Core Banking System (CBS) Logger

## Overview

The Core Banking System (CBS) Logger is a comprehensive logging system designed for financial applications. It provides secure, compliant logging that meets regulatory requirements for banking systems.

## Key Features

- **Multi-level logging** (DEBUG, INFO, WARNING, ERROR, CRITICAL, etc.)
- **Category-based organization** (APPLICATION, TRANSACTION, SECURITY, etc.)
- **Transaction logging** with before/after states
- **Security event monitoring** with severity levels
- **Audit trail** for compliance and regulatory requirements
- **PII data protection** including credit card number masking
- **Windows compatibility** through simplified implementation

## Quick Start

### Including Headers

```c
// For standard applications
#include "common/utils/cbs_logger.h"
#include "common/utils/dao_audit_logger.h"

// For Windows applications
#include "simple_cbs_logger.h"
#include "simple_dao_audit_logger.h"
```

### Basic Usage

```c
// Initialize the logger
if (!cbs_initializeLogger("logs")) {
    fprintf(stderr, "Failed to initialize CBS logger\n");
    return EXIT_FAILURE;
}

// Set log level
cbs_setLogLevel(CBS_LOG_LEVEL_INFO);

// Write a log message
cbs_writeLog(LOG_CATEGORY_APPLICATION, CBS_LOG_LEVEL_INFO, 
           "Application initialized successfully");

// Log a transaction
cbs_writeTransactionLog(
    "TX12345",           // Transaction ID
    "USER001",           // User ID
    "4111111111111111",  // Card number (will be masked)
    "ACCT001",           // Account ID
    "DEPOSIT",           // Transaction type
    100.00,              // Amount
    500.00,              // Previous balance
    600.00,              // New balance
    "SUCCESS",           // Status
    "ATM deposit"        // Details
);

// Clean up when finished
cbs_shutdownLogger();
```

## Documentation

For complete documentation, see:

- [Complete CBS Logger Guide](./docs/CBS_LOGGER_COMPLETE_GUIDE.md)
- [Documentation Index](./docs/CBS_LOGGER_DOCUMENTATION_INDEX.md)

## Test Applications

Test applications demonstrating the CBS Logger functionality:

```
bin\simple_cbs_test.exe      - Basic testing of CBS logger
bin\simple_banking_test.exe  - Simple banking operations with CBS logger
bin\cbs_banking_demo.exe     - Comprehensive demo of all CBS logging features
```

## Files & Directory Structure

```
include/common/utils/
├── cbs_logger.h       - Main CBS logger interface
├── dao_audit_logger.h - DAO audit logger interface

backend/c_backend/src/common/utils/
├── cbs_logger.c       - CBS logger implementation
├── dao_audit_logger.c - DAO audit logger implementation

tests/
├── simple_cbs_logger.h      - Simplified CBS logger interface for Windows
├── simple_cbs_logger.c      - Simplified CBS logger implementation for Windows
├── simple_dao_audit_logger.h - Simplified DAO logger interface for Windows
├── simple_dao_audit_logger.c - Simplified DAO logger implementation for Windows
```

## Windows Compatibility

The simplified implementation in the `tests` directory provides Windows compatibility without pthread dependencies.
