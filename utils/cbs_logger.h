/**
 * @file cbs_logger.h
 * @brief Core Banking System (CBS) compliant logging interface for ATM Management System
 * @version 1.0
 * @date May 10, 2025
 */

#ifndef CBS_LOGGER_H
#define CBS_LOGGER_H

#include <stdbool.h>
#include <time.h>

/**
 * @brief Log categories for CBS-compliant logging
 */
typedef enum {
    LOG_CATEGORY_APPLICATION, // Application events, startup, shutdown
    LOG_CATEGORY_TRANSACTION, // Financial transactions (deposit, withdrawal, etc.)
    LOG_CATEGORY_SECURITY,    // Security events (login, logout, auth failures)
    LOG_CATEGORY_DATABASE,    // Database operations
    LOG_CATEGORY_AUDIT,       // Audit trail entries (important for compliance)
    LOG_CATEGORY_SYSTEM       // System-level events
} LogCategory;

/**
 * @brief Log levels for CBS-compliant logging
 */
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

/**
 * @brief Data sensitivity levels for CBS-compliant logging
 */
typedef enum {
    SENSITIVITY_PUBLIC,    // Public information, no masking needed
    SENSITIVITY_INTERNAL,  // Internal information, minimal masking
    SENSITIVITY_PRIVATE,   // Private user information, requires masking
    SENSITIVITY_CRITICAL   // Critical data, extensive masking or encryption
} DataSensitivity;

/**
 * @brief Financial transaction log entry structure
 */
typedef struct {
    char transaction_id[37];    // UUID format for transaction ID
    char operation_id[50];      // Unique operation identifier
    char user_id[50];           // User or card identifier
    char transaction_type[30];  // Type of transaction
    double amount_before;       // Balance before transaction
    double amount_after;        // Balance after transaction
    double transaction_amount;  // Amount involved in transaction
    char status[20];            // Transaction status
    char details[256];          // Additional details
    time_t timestamp;           // Transaction timestamp
    char correlation_id[37];    // For tracking related operations (UUID)
} TransactionLogEntry;

/**
 * @brief Audit log entry structure
 */
typedef struct {
    char audit_id[37];          // UUID format for audit ID
    char user_id[50];           // User or card identifier
    char operation_type[30];    // Type of operation
    char resource_id[50];       // Resource being accessed
    char action[30];            // Action taken (create, read, update, delete)
    char status[20];            // Action status (success, failed)
    char before_state[512];     // State before action (JSON or structured format)
    char after_state[512];      // State after action
    char client_ip[46];         // Client IP address (IPv4 or IPv6)
    char user_agent[256];       // User agent information
    time_t timestamp;           // Action timestamp
} AuditLogEntry;

/**
 * @brief Security log entry structure
 */
typedef struct {
    char security_id[37];       // UUID format for security ID
    char user_id[50];           // User or card identifier
    char event_type[30];        // Type of security event
    char severity[20];          // Severity (high, medium, low)
    char status[20];            // Event status
    char details[256];          // Additional details
    char source_ip[46];         // Source IP address
    char target_resource[100];  // Target resource
    time_t timestamp;           // Event timestamp
} SecurityLogEntry;

/**
 * Initialize the CBS logger system
 * @param log_dir Directory to store log files
 * @return true if initialization successful, false otherwise
 */
bool cbs_initializeLogger(const char *log_dir);

/**
 * Clean up and close logger resources
 */
void cbs_closeLogger(void);

/**
 * Set the global minimum log level
 * @param level Minimum log level to record
 */
void cbs_setLogLevel(CBSLogLevel level);

/**
 * Get the current global minimum log level
 * @return Current log level
 */
CBSLogLevel cbs_getLogLevel(void);

/**
 * Write a log message with DEBUG level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeDebugLog(const char *format, ...);

/**
 * Write a log message with INFO level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeInfoLog(const char *format, ...);

/**
 * Write a log message with WARNING level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeWarningLog(const char *format, ...);

/**
 * Write a log message with ERROR level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeErrorLog(const char *format, ...);

/**
 * Write a log message with CRITICAL level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeCriticalLog(const char *format, ...);

/**
 * Write a log message with SECURITY level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeSecurityLog(const char *format, ...);

/**
 * Log a CBS transaction with full banking system details
 * @param transaction_id Unique transaction identifier
 * @param user_id User or system that initiated the transaction
 * @param card_number Card number (will be masked for security)
 * @param account_id Account identifier
 * @param transaction_type Type of transaction (deposit, withdrawal, etc.)
 * @param amount Transaction amount
 * @param prev_balance Previous account balance
 * @param new_balance New account balance after transaction
 * @param status Transaction status (success, failed, pending)
 * @param details Additional transaction details
 */
void cbs_writeTransactionLog(
    const char *transaction_id,
    const char *user_id,
    const char *card_number,
    const char *account_id,
    const char *transaction_type,
    double amount,
    double prev_balance,
    double new_balance,
    const char *status,
    const char *details);

/**
 * Write an audit log entry with CBS compliance standards
 * @param user_id User or system that performed the action
 * @param action_type Type of action performed (LOGIN, LOGOUT, CONFIG_CHANGE, etc.)
 * @param entity_type Type of entity affected (ACCOUNT, CUSTOMER, CARD, CONFIG, etc.)
 * @param entity_id ID of the entity affected
 * @param before_state State of the entity before the action (can be NULL)
 * @param after_state State of the entity after the action (can be NULL)
 * @param details Additional details about the action
 */
void cbs_writeAuditLog(
    const char *user_id,
    const char *action_type,
    const char *entity_type,
    const char *entity_id,
    const char *before_state,
    const char *after_state,
    const char *details);

/**
 * Enable or disable console output for logs
 * @param enable true to enable console output, false to disable
 */
void cbs_enableConsoleOutput(bool enable);

/**
 * Flush all buffered logs to disk
 */
void cbs_flushLogs(void);

/**
 * Set the operation ID for the current thread
 * This allows tracking a request across multiple logs
 * @param operation_id The operation ID to set
 */
void cbs_setOperationId(const char *operation_id);

/**
 * @brief Initialize legacy CBS logger system for backward compatibility
 *
 * @param log_dir Directory to store log files
 * @param enable_encryption Whether to encrypt log files
 * @param retention_days Number of days to retain log files
 * @return true if initialization successful, false otherwise
 */
bool cbs_initializeLogger(const char *log_dir, bool enable_encryption, int retention_days);

/**
 * @brief Clean up and close CBS logger resources
 */
void cbs_closeLogger(void);

/**
 * @brief Set the global minimum log level for CBS logger
 * 
 * @param level Minimum log level to record
 */
void cbs_setLogLevel(CBSLogLevel level);

/**
 * @brief Application logging function with structured data
 * 
 * @param category Log category
 * @param level Log level
 * @param operation_id Operation identifier (for tracking related logs)
 * @param user_id User or card identifier
 * @param format Format string
 * @param ... Variable arguments
 */
void cbs_logApplication(LogCategory category, CBSLogLevel level, 
                        const char *operation_id, 
                        const char *user_id, 
                        const char *format, ...);

/**
 * @brief Log a transaction with before/after state for audit trail
 * 
 * @param entry Transaction log entry
 */
void cbs_logTransaction(TransactionLogEntry *entry);

/**
 * @brief Log a security event
 * 
 * @param entry Security log entry
 */
void cbs_logSecurity(SecurityLogEntry *entry);

/**
 * @brief Log an audit entry with before/after images
 * 
 * @param entry Audit log entry
 */
void cbs_logAudit(AuditLogEntry *entry);

/**
 * @brief Log a database operation with parameter masking
 * 
 * @param operation Type of operation (SELECT, INSERT, UPDATE, DELETE)
 * @param object Target object (table name, etc.)
 * @param query SQL query with placeholders for sensitive data
 * @param params Parameter values (will be masked based on sensitivity)
 * @param sensitivity_levels Sensitivity levels for each parameter
 * @param param_count Number of parameters
 */
void cbs_logDatabase(const char *operation, 
                     const char *object,
                     const char *query,
                     const char **params,
                     DataSensitivity *sensitivity_levels,
                     int param_count);

/**
 * @brief Mask sensitive data based on data type and sensitivity
 * 
 * @param data Original data string
 * @param data_type Type of data (e.g., "CARD", "PIN", "NAME", "ACCOUNT")
 * @param sensitivity Sensitivity level
 * @param output Output buffer for masked data
 * @param output_size Size of output buffer
 */
void cbs_maskSensitiveData(const char *data, 
                          const char *data_type,
                          DataSensitivity sensitivity,
                          char *output, 
                          size_t output_size);

/**
 * @brief Generate a universally unique identifier (UUID)
 * 
 * @param output Buffer to store the UUID string (must be at least 37 bytes)
 */
void cbs_generateUUID(char *output);

/**
 * @brief Validate log integrity by checking digital signatures
 * 
 * @param log_file Path to log file
 * @return true if log integrity is verified, false otherwise
 */
bool cbs_validateLogIntegrity(const char *log_file);

/**
 * @brief Search logs with advanced filtering options
 * 
 * @param category Log category to search
 * @param start_time Start time for search range
 * @param end_time End time for search range
 * @param user_id User ID to filter by (optional, NULL to skip)
 * @param operation_type Operation type to filter by (optional, NULL to skip)
 * @param status Status to filter by (optional, NULL to skip)
 * @param max_results Maximum number of results to return
 * @param results Buffer to store results
 * @param result_count Number of results found
 * @return true if search was successful, false otherwise
 */
bool cbs_searchLogs(LogCategory category,
                   time_t start_time,
                   time_t end_time,
                   const char *user_id,
                   const char *operation_type,
                   const char *status,
                   int max_results,
                   void *results,
                   int *result_count);

/**
 * @brief Export logs to a specified format
 * 
 * @param category Log category to export
 * @param start_time Start time for export range
 * @param end_time End time for export range
 * @param format Export format ("CSV", "JSON", "XML")
 * @param output_file Path to output file
 * @return true if export was successful, false otherwise
 */
bool cbs_exportLogs(LogCategory category,
                   time_t start_time,
                   time_t end_time,
                   const char *format,
                   const char *output_file);

/**
 * @brief Rotate logs based on size or schedule
 * 
 * @param force Force rotation even if size/time thresholds not reached
 * @return true if rotation was successful, false otherwise
 */
bool cbs_rotateLogs(bool force);

/**
 * @brief Archive logs older than specified days
 * 
 * @param days Number of days, logs older than this will be archived
 * @param archive_dir Directory to store archived logs
 * @return true if archival was successful, false otherwise
 */
bool cbs_archiveLogs(int days, const char *archive_dir);

/**
 * @brief Flush all buffered logs to disk
 */
void cbs_flushLogs(void);

#endif /* CBS_LOGGER_H */
