/**
 * @file logger.h
 * @brief Logging utility for the ATM Management System
 * @version 1.0
 * @date May 3, 2025
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG,    // Detailed debug information
    LOG_LEVEL_INFO,     // Informational messages
    LOG_LEVEL_WARNING,  // Warning conditions
    LOG_LEVEL_ERROR,    // Error conditions
    LOG_LEVEL_CRITICAL, // Critical conditions
    LOG_LEVEL_SECURITY  // Security-related logs
} LogLevel;

/**
 * Initialize the logger system
 * @param log_dir Directory to store log files
 * @return true if initialization successful, false otherwise
 */
bool initializeLogger(const char *log_dir);

/**
 * Clean up and close logger resources
 */
void closeLogger(void);

/**
 * Set the global minimum log level
 * @param level Minimum log level to record
 */
void setLogLevel(LogLevel level);

/**
 * Get the current global minimum log level
 * @return Current log level
 */
LogLevel getLogLevel(void);

/**
 * Write a log message with the DEBUG level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeDebugLog(const char *format, ...);

/**
 * Write a log message with the INFO level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeInfoLog(const char *format, ...);

/**
 * Write a log message with the WARNING level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeWarningLog(const char *format, ...);

/**
 * Write a log message with the ERROR level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeErrorLog(const char *format, ...);

/**
 * Write a log message with the CRITICAL level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeCriticalLog(const char *format, ...);

/**
 * Write a log message to the security log
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeSecurityLog(const char *format, ...);

/**
 * Write a transaction log
 * @param transaction_type Transaction type
 * @param card_number Card number (masked for security)
 * @param amount Transaction amount
 * @param status Transaction status
 * @param details Additional transaction details
 */
void writeTransactionLog(const char *transaction_type, 
                        const char *card_number, 
                        double amount,
                        const char *status,
                        const char *details);

/**
 * Enable or disable console output for logs
 * @param enable true to enable console output, false to disable
 */
void enableConsoleOutput(bool enable);

/**
 * Flush all buffered logs to disk
 */
void flushLogs(void);

#endif /* LOGGER_H */