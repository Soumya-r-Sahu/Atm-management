// Mock implementation of logger functions
#include "../../../../../include/common/utils/logger.h"
#include <stdio.h>
#include <stdarg.h>

// Initialize logger
bool initializeLogger(const char *log_dir) {
    printf("MOCK: Initializing logger with directory: %s\n", log_dir);
    return true;
}

// Clean up logger
void closeLogger(void) {
    printf("MOCK: Closing logger\n");
}

// Set global log level
void setLogLevel(LogLevel level) {
    printf("MOCK: Setting log level to %d\n", level);
}

// Get current log level
LogLevel getLogLevel(void) {
    return LOG_LEVEL_INFO;
}

// Write debug log
void writeDebugLog(const char *format, ...) {
    printf("MOCK: Debug log: (message suppressed)\n");
}

// Write info log
void writeInfoLog(const char *format, ...) {
    printf("MOCK: Info log: (message suppressed)\n");
}

// Write warning log
void writeWarningLog(const char *format, ...) {
    printf("MOCK: Warning log: (message suppressed)\n");
}

// Write error log
void writeErrorLog(const char *format, ...) {
    printf("MOCK: Error log: (message suppressed)\n");
}

// Write critical log
void writeCriticalLog(const char *format, ...) {
    printf("MOCK: Critical log: (message suppressed)\n");
}

// Write security log
void writeSecurityLog(const char *format, ...) {
    printf("MOCK: Security log: (message suppressed)\n");
}

// Write transaction log
void writeTransactionLog(const char *transaction_type, 
                        const char *card_number, 
                        double amount,
                        const char *status,
                        const char *details) {
    printf("MOCK: Transaction log: %s for card %s - $%.2f (%s) - %s\n", 
           transaction_type, card_number, amount, status, details);
}

// Enable console output
void enableConsoleOutput(bool enable) {
    printf("MOCK: %s console output\n", enable ? "Enabling" : "Disabling");
}

// Flush logs
void flushLogs(void) {
    printf("MOCK: Flushing logs\n");
}
