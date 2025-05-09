# Core Banking System (CBS) Logger - Complete Documentation

## Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [Architecture](#architecture)
4. [Installation & Setup](#installation--setup)
5. [API Reference](#api-reference)
6. [Developer Usage Guide](#developer-usage-guide)
7. [Testing](#testing)
8. [Windows Compatibility](#windows-compatibility)
9. [Best Practices](#best-practices)
10. [Common Issues & Troubleshooting](#common-issues--troubleshooting)

## Overview

The Core Banking System (CBS) logger provides comprehensive logging functionality designed specifically for financial applications. It meets regulatory requirements for banking systems, including security standards, audit trails, and data protection regulations.

The logger system is an essential component of the ATM Management Project that ensures:
- Complete transaction history for financial operations
- Detailed security monitoring and alerting
- Comprehensive audit trails for compliance
- Protection of sensitive data

## Features

### Core Logging Capabilities
- **Multi-Level Logging**: 
  - DEBUG: Detailed development information
  - INFO: Informational messages about system operation
  - WARNING: Potential issues that don't prevent system operation
  - ERROR: Error conditions that should be addressed
  - CRITICAL: Critical conditions that require immediate attention
  - SECURITY: Security-related events
  - ALERT: Action must be taken immediately
  - EMERGENCY: System is unusable

- **Category-Based Organization**:
  - APPLICATION: General application events
  - TRANSACTION: Financial transaction details
  - SECURITY: Security-related events
  - DATABASE: Database operations
  - AUDIT: Audit trail entries
  - SYSTEM: System-level events
  - ERROR: Error events

### Advanced Features
- **Transaction Logging**:
  - Records full transaction details including:
    - Transaction ID and type
    - Before/after account balances
    - Timestamp and user information
    - Transaction status and result

- **Security Event Monitoring**:
  - Authentication attempts (success/failure)
  - Authorization decisions
  - Security control changes
  - Suspicious activity detection

- **Audit Trail**:
  - Complete state changes for all operations
  - User actions tracking
  - System configuration changes
  - Seven-year retention for regulatory compliance

- **Data Protection**:
  - PII (Personally Identifiable Information) data masking
  - Credit card number obfuscation (PCI DSS compliance)
  - Sensitive data encryption

- **System Monitoring**:
  - Log rotation and management
  - Size limiting and archiving
  - Performance metrics

## Architecture

The CBS Logger implements a layered architecture:

```
+------------------------------------------+
|           Application Layer              |
+------------------------------------------+
                    │
                    ▼
+------------------------------------------+
|       CBS Logger Interface Layer         |
|   (cbs_logger.h / simple_cbs_logger.h)   |
+------------------------------------------+
                    │
                    ▼
+------------------------------------------+
|            Specialized Loggers           |
|   Transaction │ Security │ Audit │ Error |
+------------------------------------------+
                    │
                    ▼
+------------------------------------------+
|            Storage Backends              |
|    File-based  │  Centralized  │  SIEM   |
+------------------------------------------+
```

### Key Components

1. **CBS Logger Core**:
   - Handles basic logging functions
   - Manages log levels and categories
   - Provides thread safety and synchronization

2. **Specialized Loggers**:
   - Transaction Logger: For financial transactions
   - Security Logger: For security events
   - Audit Logger: For compliance and auditing
   - DAO Audit Logger: For data access operations

3. **Storage Backend**:
   - File-based logging with rotation
   - Directory structure for log organization
   - Support for future integration with centralized logging

## Installation & Setup

### Prerequisites
- C compiler (GCC recommended)
- Standard C libraries
- File system write permissions

### Integration Steps

1. Include the necessary header files in your application:
```c
#include "common/utils/cbs_logger.h"       // For standard CBS logging
#include "common/utils/dao_audit_logger.h" // For DAO-specific audit logging
```

2. Initialize the logging system at application startup:
```c
// Initialize the CBS logger
if (!cbs_initializeLogger("logs")) {
    fprintf(stderr, "Failed to initialize CBS logger\n");
    return EXIT_FAILURE;
}

// Initialize the DAO audit logger if needed
if (!init_dao_audit_logger("logs", 7)) {
    fprintf(stderr, "Failed to initialize DAO audit logger\n");
    cbs_shutdownLogger();
    return EXIT_FAILURE;
}

// Set desired log level
cbs_setLogLevel(CBS_LOG_LEVEL_INFO);
```

3. Shut down the logging system before application exit:
```c
// Clean up logging resources
close_dao_audit_logger();
cbs_shutdownLogger();
```

### Directory Structure
The logger creates the following directory structure:
```
logs/
├── application.log  - General application events
├── transactions.log - Financial transaction details
├── security.log     - Security-related events
├── audit.log        - Full audit trail
├── error.log        - Error messages and exceptions
```

## API Reference

### Core Logging Functions

#### Initialization and Configuration
```c
// Initialize the CBS logger system
bool cbs_initializeLogger(const char *log_dir);

// Set the minimum log level
void cbs_setLogLevel(CBSLogLevel level);

// Get the current log level
CBSLogLevel cbs_getLogLevel(void);

// Clean up logger resources
void cbs_shutdownLogger(void);
```

#### General Logging
```c
// Write a log with category and level
void cbs_writeLog(LogCategory category, CBSLogLevel level, const char *format, ...);
```

#### Specialized Logging
```c
// Log a financial transaction
void cbs_writeTransactionLog(
    const char *transaction_id,
    const char *user_id,
    const char *card_number,  // Will be masked
    const char *account_id,
    const char *transaction_type,
    double amount,
    double prev_balance,
    double new_balance,
    const char *status,
    const char *details);

// Log a security event
void cbs_writeSecurityLog(
    const char *user_id,
    const char *event_type,
    const char *severity,
    const char *status,
    const char *resource_id,
    const char *details,
    const char *source_ip);

// Log an audit entry
void cbs_writeAuditLog(
    const char *user_id,
    const char *operation_type,
    const char *resource_type,
    const char *resource_id,
    const char *before_state,
    const char *after_state,
    const char *details);
```

### DAO Audit Logger Functions

```c
// Initialize the DAO audit logger
bool init_dao_audit_logger(const char* log_directory, int retention_days);

// Close the DAO audit logger
void close_dao_audit_logger(void);

// Record transaction audit
void record_transaction_audit(
    const char* user_id,
    int cardNumber,
    const char* transaction_type,
    float amount,
    float old_balance,
    float new_balance,
    bool success);

// Record card operation audit
void record_card_operation_audit(
    const char* user_id,
    int cardNumber,
    const char* operation_type,
    const char* before_status,
    const char* after_status,
    bool success);

// Record database operation audit
void record_db_operation_audit(
    const char* operation_type,
    const char* table_name,
    const char* record_id,
    const char* before_state,
    const char* after_state,
    bool success,
    const char* error_message);

// Record authentication audit
void record_auth_audit(
    const char* user_id,
    const char* auth_type,
    bool success,
    const char* ip_address,
    const char* device_info);
```

## Developer Usage Guide

### Logging Levels - When to Use What

| Level | When to Use |
|-------|-------------|
| DEBUG | Development information, verbose details useful during development |
| INFO | Normal operational messages, general workflow tracking |
| WARNING | Potential issues that don't stop system operation but should be reviewed |
| ERROR | Error conditions that prevent a specific operation but not the whole application |
| CRITICAL | Critical conditions requiring immediate attention |
| SECURITY | Security-related events regardless of severity |
| ALERT | Conditions requiring immediate action |
| EMERGENCY | System is unusable |

### Logging Categories - Organization

| Category | Purpose |
|----------|---------|
| APPLICATION | General application events like startup/shutdown |
| TRANSACTION | Financial transactions |
| SECURITY | Security-related events |
| DATABASE | Database operations and queries |
| AUDIT | Compliance audit trail entries |
| SYSTEM | System-level events (hardware, OS interaction) |
| ERROR | Error events from any part of the system |

### Example: Logging an ATM Withdrawal

```c
void process_withdrawal(const char* user_id, const char* card_number, 
                        const char* account_id, double amount) {
    // Generate transaction ID
    char transaction_id[20];
    sprintf(transaction_id, "TX%ld%05d", time(NULL), rand() % 99999);
    
    // Get current balance (sample code)
    double current_balance = get_account_balance(account_id);
    
    // Log the start of transaction
    cbs_writeLog(LOG_CATEGORY_TRANSACTION, CBS_LOG_LEVEL_INFO,
                "Starting withdrawal transaction %s for $%.2f", 
                transaction_id, amount);
    
    // Check balance
    if (current_balance < amount) {
        // Log insufficient funds
        cbs_writeLog(LOG_CATEGORY_ERROR, CBS_LOG_LEVEL_WARNING,
                    "Insufficient funds for withdrawal: $%.2f < $%.2f",
                    current_balance, amount);
        return;
    }
    
    // Process withdrawal (sample code)
    double new_balance = current_balance - amount;
    update_account_balance(account_id, new_balance);
    
    // Log the transaction details
    cbs_writeTransactionLog(
        transaction_id,
        user_id,
        card_number,  // Will be automatically masked in logs
        account_id,
        "WITHDRAWAL",
        amount,
        current_balance,
        new_balance,
        "SUCCESS",
        "ATM withdrawal transaction"
    );
    
    // Also record in the audit system
    record_transaction_audit(
        user_id,
        atoi(card_number),  // Card number as int
        "WITHDRAWAL",
        amount,
        current_balance,
        new_balance,
        true  // success
    );
}
```

### Example: Logging a Security Event

```c
void process_login_attempt(const char* user_id, bool success, const char* ip_address) {
    // Record the login attempt
    const char* status = success ? "SUCCESS" : "FAILURE";
    const char* severity = success ? "LOW" : "MEDIUM";
    
    cbs_writeSecurityLog(
        user_id,
        "LOGIN",
        severity,
        status,
        "ATM_TERMINAL",
        success ? "Normal login" : "Failed login attempt",
        ip_address
    );
    
    // Also record in the audit system
    record_auth_audit(
        user_id,
        "PIN",
        success,
        ip_address,
        "ATM Terminal"
    );
    
    // Log to application log
    cbs_writeLog(LOG_CATEGORY_APPLICATION, 
                success ? CBS_LOG_LEVEL_INFO : CBS_LOG_LEVEL_WARNING,
                "User %s login %s from %s", 
                user_id, status, ip_address);
}
```

## Testing

### Test Programs

The CBS Logger system includes multiple test programs to verify functionality:

1. **simple_cbs_test**: Tests basic CBS logger functionality
   ```
   bin\simple_cbs_test.exe
   ```

2. **simple_banking_test**: Tests banking operations with simplified logging
   ```
   bin\simple_banking_test.exe
   ```

3. **cbs_banking_demo**: Comprehensive demo of banking operations with full CBS logging
   ```
   bin\cbs_banking_demo.exe
   ```

### Sample Test Output

Running the `cbs_banking_demo.exe` demonstrates the following:
- Account creation with proper logging
- Financial transactions (deposit, withdrawal, transfer)
- Security event logging (login attempts, card operations)
- PII data masking
- Complete audit trail

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

## Windows Compatibility

The original CBS Logger relies on POSIX features like pthread.h which are not directly available on Windows. Simplified versions (`simple_cbs_logger.c/h` and `simple_dao_audit_logger.c/h`) are provided for Windows compatibility.

### Key Differences in Windows Version

1. **No Thread Safety**: The simplified version lacks mutex protection
2. **Directory Creation**: Uses Windows-specific commands for directory creation
3. **Simplified File Handling**: Less sophisticated file rotation
4. **No Log Compression**: Simplified log management

### Using the Windows-Compatible Version

Include the simplified headers instead of the standard ones:

```c
#include "simple_cbs_logger.h"
#include "simple_dao_audit_logger.h"
```

Build commands:
```
gcc -o bin\cbs_banking_demo.exe tests\cbs_banking_demo.c tests\simple_cbs_logger.c tests\simple_dao_audit_logger.c -I. -Itests -Wall -O2
```

## Best Practices

1. **Always Initialize and Shutdown**
   ```c
   // At application startup
   cbs_initializeLogger("logs");
   
   // Before application exit
   cbs_shutdownLogger();
   ```

2. **Use Appropriate Log Levels**
   - Don't overuse high severity levels
   - Use DEBUG for developer information
   - Use INFO for normal operations
   - Use WARNING for potential issues
   - Use ERROR for operation failures
   - Use CRITICAL for system failures

3. **Include Contextual Information**
   - User ID or session information
   - Transaction identifiers
   - Related entity IDs (account, card)
   - Operation status and error codes

4. **PII Data Handling**
   - Never log full card numbers
   - Use provided masking functions
   - Be careful with personal information
   - Follow data protection regulations

5. **Transaction Logging**
   - Always include before/after states
   - Generate unique transaction IDs
   - Record both success and failure cases
   - Include detailed error information for failures

6. **Audit Trail**
   - Log all state-changing operations
   - Include the actor (user) information
   - Record timestamps with precision
   - Maintain data for regulatory periods (7+ years)

## Common Issues & Troubleshooting

### Log Files Not Being Created

**Problem**: Log files aren't being created at the expected location.

**Solutions**:
- Ensure the application has write permissions to the log directory
- Check that the directory path exists or can be created
- Verify initialization was successful (check return value)

```c
if (!cbs_initializeLogger("logs")) {
    fprintf(stderr, "Failed to initialize CBS logger\n");
    // Handle the error
}
```

### Missing Transaction Information

**Problem**: Transactions are logged without all required information.

**Solution**: Always provide complete information to the transaction logging functions:

```c
// Complete transaction log
cbs_writeTransactionLog(
    "TX123456",          // Transaction ID
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
```

### Thread Safety Issues

**Problem**: Logs are corrupted or incomplete when multiple threads are writing.

**Solutions**:
- Use the full CBS Logger implementation with pthread support on platforms that support it
- In Windows, ensure serialized access to the logger

### Excessive Log Size

**Problem**: Log files grow too large.

**Solution**: 
- Use appropriate log levels to control verbosity
- Enable log rotation with reasonable size limits
- Implement log file cleanup routines

### Card Numbers Not Masked

**Problem**: Sensitive information like card numbers appears unmasked in logs.

**Solution**:
- Always use the cbs_writeTransactionLog() function which applies masking
- For custom logging, use the provided masking functions
- Never log sensitive information with standard printf()

---

*This documentation consolidates information from CBS_LOGGER_DOCUMENTATION.md, cbs_logging_architecture.md, and cbs_logging_developer_guide.md into a single comprehensive reference.*
