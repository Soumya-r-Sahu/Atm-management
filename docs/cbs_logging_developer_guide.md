# CBS Logging System - Developer Guide

## Introduction

This guide provides practical instructions for developers on how to use the Core Banking System (CBS) compliant logging system in the ATM project. Following these guidelines ensures consistent logging across the application and compliance with banking industry standards.

## Getting Started

### Including the Required Headers

```c
#include "common/utils/cbs_logger.h"       // For CBS logging functions
#include "common/utils/dao_audit_logger.h" // For DAO audit logging
```

### Initializing the Loggers

Initialize the CBS logger system at application startup:

```c
// Initialize CBS logger with log directory
if (!cbs_initializeLogger("./logs")) {
    fprintf(stderr, "Failed to initialize CBS logger\n");
    return EXIT_FAILURE;
}

// Initialize DAO audit logger with log directory and retention days
if (!init_dao_audit_logger("./logs", 7)) {
    fprintf(stderr, "Failed to initialize DAO audit logger\n");
    cbs_shutdownLogger();
    return EXIT_FAILURE;
}

// Set the desired log level
cbs_setLogLevel(CBS_LOG_LEVEL_INFO);
```

### Shutting Down the Loggers

Always shut down the loggers properly before application exit:

```c
// Close both loggers
close_dao_audit_logger();
cbs_shutdownLogger();
```

## Basic Logging

### Writing General Log Messages

For general application logs:

```c
cbs_writeLog(LOG_CATEGORY_APPLICATION, CBS_LOG_LEVEL_INFO, 
             "Application started successfully");

cbs_writeLog(LOG_CATEGORY_SYSTEM, CBS_LOG_LEVEL_WARNING, 
             "Low disk space detected: %d MB remaining", remaining_mb);

cbs_writeLog(LOG_CATEGORY_DATABASE, CBS_LOG_LEVEL_ERROR, 
             "Failed to connect to database: %s", error_message);
```

### Using the Right Log Categories

Choose the appropriate log category based on the content:

- `LOG_CATEGORY_APPLICATION` - General application events
- `LOG_CATEGORY_TRANSACTION` - Financial transactions
- `LOG_CATEGORY_SECURITY` - Security-related events
- `LOG_CATEGORY_DATABASE` - Database operations
- `LOG_CATEGORY_AUDIT` - Audit trail entries
- `LOG_CATEGORY_SYSTEM` - System-level events

### Choosing the Right Log Level

Select log levels according to severity:

- `CBS_LOG_LEVEL_DEBUG` - Detailed debugging information
- `CBS_LOG_LEVEL_INFO` - Normal operational messages
- `CBS_LOG_LEVEL_WARNING` - Warning conditions
- `CBS_LOG_LEVEL_ERROR` - Error conditions
- `CBS_LOG_LEVEL_CRITICAL` - Critical conditions
- `CBS_LOG_LEVEL_SECURITY` - Security-specific events
- `CBS_LOG_LEVEL_ALERT` - Immediate action required
- `CBS_LOG_LEVEL_EMERGENCY` - System is unusable

## Transaction Logging

### Recording Financial Transactions

Always record both before and after states:

```c
// Record a transaction with audit trail
record_transaction_audit(
    user_id,              // User or customer ID
    card_number,          // Card number (will be masked)
    "DEPOSIT",            // Transaction type
    amount,               // Transaction amount
    old_balance,          // Balance before transaction
    new_balance,          // Balance after transaction
    success               // Transaction success flag
);
```

### Transaction Types

Use consistent transaction type strings:

- `"DEPOSIT"` - Money deposit transactions
- `"WITHDRAWAL"` - Money withdrawal transactions
- `"BALANCE_CHECK"` - Balance inquiry
- `"TRANSFER"` - Funds transfer
- `"PAYMENT"` - Bill payment
- `"MINI_STATEMENT"` - Mini statement request

## Security Logging

### Recording Security Events

```c
cbs_writeSecurityLog(
    user_id,                      // User ID
    "AUTH_FAILURE",               // Event type
    "HIGH",                       // Severity (HIGH, MEDIUM, LOW)
    "FAILED",                     // Status
    "PIN_ATTEMPT",                // Target resource
    "Failed PIN authentication",  // Details
    client_ip                     // IP address
);
```

### Security Event Types

Use consistent security event type strings:

- `"AUTH_SUCCESS"` - Successful authentication
- `"AUTH_FAILURE"` - Failed authentication
- `"CARD_BLOCKED"` - Card has been blocked
- `"ACCESS_DENIED"` - Access denied to resource
- `"INVALID_INPUT"` - Invalid user input detected
- `"PERMISSION_VIOLATION"` - User attempted unauthorized action

## Card Operation Logging

```c
record_card_operation_audit(
    user_id,              // User ID
    card_number,          // Card number
    "BLOCK",              // Operation type
    "ACTIVE",             // Status before operation
    "BLOCKED",            // Status after operation
    success               // Operation success flag
);
```

## Database Operation Logging

```c
record_db_operation_audit(
    "UPDATE",                                 // Operation type
    "ACCOUNTS",                               // Table name
    record_id,                                // Record identifier
    "{\"balance\":1000.00,\"status\":\"A\"}", // Before state (JSON)
    "{\"balance\":1200.00,\"status\":\"A\"}", // After state (JSON)
    success,                                  // Operation success flag
    error_message                             // Error message or NULL
);
```

## Authentication Logging

```c
record_auth_audit(
    user_id,             // User ID
    "PIN",               // Authentication type
    success,             // Authentication success flag
    client_ip,           // Client IP address
    device_info          // Device information
);
```

## Proper PII Handling

### What to Mask

Always mask the following information:
- Card numbers (only show last 4 digits)
- Social security numbers
- Full addresses
- Phone numbers
- Account numbers
- Passwords

### Using Direct Masking

For custom PII masking:

```c
char masked_card[20];
mask_card_number(card_number, masked_card);
printf("Processing card: %s\n", masked_card);
```

## Integration with DAO Layer

For transaction manager integration, use the enhanced_transaction_log helper:

```c
enhanced_transaction_log(
    card_number,
    username,
    "Deposit",
    amount,
    old_balance,
    new_balance,
    success
);
```

## Best Practices

1. **Be Consistent** - Use consistent transaction types, event types, and status codes

2. **Provide Context** - Include enough information to understand what happened:
   ```c
   cbs_writeLog(LOG_CATEGORY_APPLICATION, CBS_LOG_LEVEL_ERROR, 
                "Failed to process transaction ID %s: %s", 
                transaction_id, error_message);
   ```

3. **Follow State Changes** - Always log both before and after states for important operations

4. **Use Appropriate Log Levels** - Don't log everything as ERROR or INFO

5. **Protect Sensitive Data** - Never log full card numbers, passwords, or PINs

6. **Include Operation IDs** - For related operations to enable correlation:
   ```c
   cbs_writeLog(LOG_CATEGORY_DATABASE, CBS_LOG_LEVEL_INFO, 
                "Operation %s: Executing query", operation_id);
   ```

7. **Add Error Context** - Include error codes and possible solutions when logging errors

## Common Mistakes to Avoid

1. **Logging Sensitive Information** - Never log full card numbers, PINs, or passwords

2. **Inconsistent Format** - Use the provided functions rather than custom formats

3. **Missing State Changes** - Always log before/after states for important data

4. **Wrong Log Level** - Don't use ERROR for informational messages

5. **Excessive Logging** - Don't log routine operations at DEBUG level in production

6. **Insufficient Context** - "Error occurred" is not helpful; include what, why, and where

7. **Ignoring Return Values** - Always check if logging initialization succeeded

## Troubleshooting

If logs aren't appearing where expected:

1. Check that `cbs_initializeLogger()` was called successfully
2. Verify the log level is appropriate (e.g., DEBUG logs won't appear if level is set to INFO)
3. Ensure log directory exists and is writable
4. Check that logging calls use correct category and level
5. Ensure loggers are properly shut down

## Further Reading

- Review the comprehensive log architecture in `docs/cbs_logging_architecture.md`
- See example usage in `tests/test_cbs_logger.c`
- Consult regulatory requirements for specific logging needs
