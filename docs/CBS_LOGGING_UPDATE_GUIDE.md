# CBS Logging System Update Guide

This guide explains how to update the CBS logging system to use the new structured logging directories.

## New Log Directory Structure

```
/logs/
├── transactions/      # All transaction-related logs
│   ├── atm_transactions.log
│   ├── upi_transactions.log
│   └── web_transactions.log
├── security/          # Security and authentication logs
│   ├── admin_auth.log
│   ├── customer_auth.log
│   └── security_events.log
├── errors/            # Error logs
│   ├── application_errors.log
│   ├── database_errors.log
│   └── network_errors.log
└── audit/             # Audit trail logs
    ├── system_audit.log
    ├── database_audit.log
    └── user_actions.log
```

## Logger Configuration Updates

### 1. Update Logger Initialization

In `cbs_logger.c` (or your logger implementation), update the initialization function to create these directories if they don't exist:

```c
bool cbs_initializeLogger(const char* baseLogDirectory) {
    // Create main log directory if it doesn't exist
    if (!ensureDirectoryExists(baseLogDirectory)) {
        return false;
    }
    
    // Create required subdirectories
    const char* subDirs[] = {
        "transactions",
        "security",
        "errors",
        "audit"
    };
    
    for (int i = 0; i < sizeof(subDirs)/sizeof(subDirs[0]); i++) {
        char subdirPath[256];
        snprintf(subdirPath, sizeof(subdirPath), "%s/%s", baseLogDirectory, subDirs[i]);
        if (!ensureDirectoryExists(subdirPath)) {
            return false;
        }
    }
    
    // Complete remaining initialization
    ...
}
```

### 2. Update Log File Paths

Update the path construction for each log file type:

```c
void cbs_writeTransactionLog(...) {
    char logFilePath[256];
    snprintf(logFilePath, sizeof(logFilePath), "%s/transactions/%s", 
             logDirectory, getTransactionLogFilename());
    
    // Continue with logging...
}

void cbs_writeSecurityLog(...) {
    char logFilePath[256];
    snprintf(logFilePath, sizeof(logFilePath), "%s/security/%s", 
             logDirectory, getSecurityLogFilename());
    
    // Continue with logging...
}

void cbs_writeErrorLog(...) {
    char logFilePath[256];
    snprintf(logFilePath, sizeof(logFilePath), "%s/errors/%s", 
             logDirectory, getErrorLogFilename());
    
    // Continue with logging...
}

void cbs_writeAuditLog(...) {
    char logFilePath[256];
    snprintf(logFilePath, sizeof(logFilePath), "%s/audit/%s", 
             logDirectory, getAuditLogFilename());
    
    // Continue with logging...
}
```

### 3. Update Helper Functions

If you're using helper functions to determine log filenames, update them too:

```c
const char* getTransactionLogFilename() {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    static char filename[50];
    strftime(filename, sizeof(filename), "transactions_%Y-%m-%d.log", tm_info);
    
    return filename;
}

// Similar for other log types...
```

## Testing Updates

After making these changes, test the logging system to ensure:

1. All directories are created correctly
2. Logs are written to the correct locations
3. Existing code can still find and read logs
4. Log rotation still works correctly

## Migration Steps

1. Create a backup of your current logs
2. Update the logger code as described above
3. Test logging functionality in a development environment
4. Deploy the updated logger to production
5. Monitor for any issues or regressions
