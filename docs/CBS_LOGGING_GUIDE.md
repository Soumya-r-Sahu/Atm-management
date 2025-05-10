# CBS Logging System Guide

## Overview

The CBS Logging System provides a comprehensive logging solution for banking applications with:

- Multi-level logging (DEBUG to EMERGENCY)
- Category-based organization
- Secure transaction logging
- PII data protection
- Audit compliance features

## Implementations

The system offers two implementations:

1. **Full Implementation**: `backend/c_backend/src/common/utils/cbs_logger.c`
   - Thread-safe with pthread support
   - Comprehensive banking compliance features
   - For production environments

2. **Simplified Implementation**: `tests/simple_cbs_logger.c`
   - Windows-compatible without pthread dependencies
   - Core functionality only
   - For testing and Windows development

## Logger Setup

### Initialization

```c
// Include the appropriate header
#include "common/utils/cbs_logger.h"       // Full implementation
// OR
#include "simple_cbs_logger.h"             // Simplified implementation

// Initialize the logger
if (!cbs_initializeLogger("./logs")) {
    fprintf(stderr, "Failed to initialize CBS logger\n");
    return EXIT_FAILURE;
}

// Set log level (optional)
cbs_setLogLevel(CBS_LOG_LEVEL_INFO);
```

### Cleanup

```c
// Always shut down properly before application exit
cbs_shutdownLogger();
```

## Basic Logging

### Standard Log Messages

```c
// Basic log message
cbs_writeLog(LOG_CATEGORY_APPLICATION, CBS_LOG_LEVEL_INFO, 
           "Application started successfully");

// With parameters
cbs_writeLog(LOG_CATEGORY_DATABASE, CBS_LOG_LEVEL_ERROR, 
           "Database error %d: %s", error_code, error_message);
```

### Convenience Macros

```c
// These macros automatically include file/line information
CBS_LOG_INFO(LOG_CATEGORY_APPLICATION, "Application initialized");
CBS_LOG_ERROR(LOG_CATEGORY_DATABASE, "Connection failed: %s", error_message);
CBS_LOG_CRITICAL(LOG_CATEGORY_SYSTEM, "Out of memory");
```

## Specialized Logging

### Transaction Logging

```c
// Log complete transaction details
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
```

### Security Event Logging

```c
// Log security events
cbs_writeSecurityLog(
    "USER123",           // User ID
    "AUTH_FAILURE",      // Event type
    "HIGH",              // Severity
    "FAILED",            // Status
    "PIN_ATTEMPT",       // Target resource
    "3rd failed attempt",// Details
    "192.168.1.100"      // Source IP
);
```

## DAO Audit Logging

For database and transaction auditing:

```c
#include "common/utils/dao_audit_logger.h"  // Full implementation
// OR
#include "simple_dao_audit_logger.h"        // Simplified implementation

// Initialize
if (!init_dao_audit_logger("./logs", 7)) {
    fprintf(stderr, "Failed to initialize DAO audit logger\n");
    return EXIT_FAILURE;
}

// Log transaction with audit trail
record_transaction_audit(
    "USER001",           // User ID
    "4111111111111111",  // Card number (will be masked)
    "DEPOSIT",           // Transaction type
    100.00,              // Amount
    500.00,              // Old balance
    600.00,              // New balance
    true                 // Success flag
);

// Log database operations
record_db_operation_audit(
    "UPDATE",            // Operation type
    "ACCOUNTS",          // Table name
    "ACCT001",           // Record identifier
    "{\"balance\":500}", // Before state (JSON)
    "{\"balance\":600}", // After state (JSON)
    true,                // Success flag
    NULL                 // Error message (if any)
);

// Always close properly
close_dao_audit_logger();
```

## PII Data Protection

### Card Number Masking

```c
char masked_card[25];
cbs_maskCardNumber("4111111111111111", masked_card, sizeof(masked_card));
// Result: "************1111"

// Automatic masking occurs in transaction logs:
cbs_writeTransactionLog("TX1", "USER1", "4111111111111111", "ACCT1", 
                       "WITHDRAWAL", 50.0, 100.0, 50.0, "SUCCESS", "ATM");
// Card number is masked in logs
```

### Other PII Masking

```c
char masked_ssn[20];
cbs_maskPII("123-45-6789", masked_ssn, sizeof(masked_ssn));
// Result: "XXX-XX-6789"
```

## Best Practices

### Log Categories

Choose the appropriate category:

| Category | Usage |
|----------|-------|
| `LOG_CATEGORY_APPLICATION` | General app events (startup, shutdown) |
| `LOG_CATEGORY_TRANSACTION` | Financial transactions |  
| `LOG_CATEGORY_SECURITY` | Security events (login attempts, etc.) |
| `LOG_CATEGORY_DATABASE` | Database operations |
| `LOG_CATEGORY_AUDIT` | Audit trail entries |
| `LOG_CATEGORY_SYSTEM` | System events (resources, config) |

### Log Levels

Select levels according to severity:

| Level | Usage |
|-------|-------|
| `CBS_LOG_LEVEL_DEBUG` | Detailed debugging information |
| `CBS_LOG_LEVEL_INFO` | Normal operational messages |
| `CBS_LOG_LEVEL_WARNING` | Potential issues |
| `CBS_LOG_LEVEL_ERROR` | Error conditions |
| `CBS_LOG_LEVEL_CRITICAL` | Critical conditions |
| `CBS_LOG_LEVEL_SECURITY` | Security-specific events |
| `CBS_LOG_LEVEL_ALERT` | Immediate action required |
| `CBS_LOG_LEVEL_EMERGENCY` | System is unusable |

### Transaction Types

Use consistent transaction type strings:

- `"DEPOSIT"` - Money deposit
- `"WITHDRAWAL"` - Money withdrawal
- `"BALANCE_CHECK"` - Balance inquiry
- `"TRANSFER"` - Funds transfer
- `"PAYMENT"` - Bill payment
- `"MINI_STATEMENT"` - Statement request

### Security Events

Use consistent security event types:

- `"AUTH_SUCCESS"` - Successful authentication
- `"AUTH_FAILURE"` - Failed authentication
- `"CARD_BLOCKED"` - Card has been blocked
- `"ACCESS_DENIED"` - Access denied to resource
- `"INVALID_INPUT"` - Invalid user input detected
- `"PERMISSION_VIOLATION"` - Unauthorized action attempt

## Troubleshooting

Common issues and solutions:

1. **No log files created**
   - Check directory permissions
   - Verify log directory exists
   - Ensure logger was successfully initialized

2. **Missing log entries**
   - Verify log level (e.g., DEBUG logs won't appear if level is set to INFO)
   - Check that correct category is being used

3. **Poor performance**
   - Too much DEBUG logging in production
   - Consider using the async logger option for high-volume environments

4. **Log file size issues**
   - Configure max file size and rotation in system config
   - Enable log compression if supported

---

## Navigation

- [Documentation Index](./README.md)
- [Project Documentation](./CBS_PROJECT_DOCUMENTATION.md)
- [Function Reference](./CBS_FUNCTION_REFERENCE.md)
- [Build Guide](./CBS_BUILD_GUIDE.md)
- [Testing Plan](./CBS_TESTING_PLAN.md)
