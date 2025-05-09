/**
 * @file cbs_logger.c
 * @brief Core Banking System (CBS) compliant logging implementation for ATM Management System
 * @version 1.0
 * @date May 10, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include "common/utils/cbs_logger.h"

/* Log file configuration */
#define MAX_LOG_MESSAGE_LENGTH 4096
#define MAX_LOG_SIZE_DEFAULT 10485760  // 10 MB
#define MAX_LOG_RETENTION_DAYS 7       // Keep logs for 7 days
#define MAX_AUDIT_RETENTION_DAYS 2555  // Keep audit logs for 7 years (regulatory requirement)

/* Log file handles */
static FILE* application_log_file = NULL;
static FILE* transaction_log_file = NULL;
static FILE* security_log_file = NULL;
static FILE* audit_log_file = NULL;
static FILE* error_log_file = NULL;
static FILE* debug_log_file = NULL;

/* Log directory */
static char log_directory[256] = "logs";

/* Current log level */
static CBSLogLevel current_log_level = CBS_LOG_LEVEL_INFO;

/* Console output flag */
static bool console_output_enabled = true;

/* Thread safety mutex */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Audit trail settings */
static char instance_id[37] = {0};  // UUID for instance identification
static unsigned long sequence_number = 0; // Sequential log ID

/* Forward declarations of internal functions */
static void rotate_log_if_needed(FILE** log_file, const char* filename);
static void get_current_time_str(char* buffer, size_t size);
static const char* get_log_level_str(CBSLogLevel level);
static char* format_log_message(CBSLogLevel level, const char* format, va_list args);
static void internal_write_log(FILE* log_file, CBSLogLevel level, const char* format, va_list args);
static void mask_pii_data(char* text, const char* field_name);
static bool ensure_directory_exists(const char* dir_path);
static void generate_instance_id(void);
static char* get_timestamp_with_ms(void);
static void delete_old_logs(const char* log_dir, int retention_days);
static char* get_operation_id(void);

/**
 * Initialize the CBS logger system
 * @param log_dir Directory to store log files
 * @return true if initialization successful, false otherwise
 */
bool cbs_initializeLogger(const char *log_dir) {
    pthread_mutex_lock(&log_mutex);
    
    // Set log directory
    if (log_dir) {
        strncpy(log_directory, log_dir, sizeof(log_directory) - 1);
        log_directory[sizeof(log_directory) - 1] = '\0';
    }
    
    // Ensure the log directory exists
    if (!ensure_directory_exists(log_directory)) {
        pthread_mutex_unlock(&log_mutex);
        return false;
    }
    
    // Open log files
    char path[512];
    
    // Application log
    snprintf(path, sizeof(path), "%s/application.log", log_directory);
    application_log_file = fopen(path, "a");
    
    // Transaction log
    snprintf(path, sizeof(path), "%s/transactions.log", log_directory);
    transaction_log_file = fopen(path, "a");
    
    // Security log
    snprintf(path, sizeof(path), "%s/security.log", log_directory);
    security_log_file = fopen(path, "a");
    
    // Audit log
    snprintf(path, sizeof(path), "%s/audit.log", log_directory);
    audit_log_file = fopen(path, "a");
    
    // Error log
    snprintf(path, sizeof(path), "%s/error.log", log_directory);
    error_log_file = fopen(path, "a");
    
    // Debug log
    snprintf(path, sizeof(path), "%s/debug.log", log_directory);
    debug_log_file = fopen(path, "a");
    
    // Check if all files were opened successfully
    if (!application_log_file || !transaction_log_file || !security_log_file || 
        !audit_log_file || !error_log_file || !debug_log_file) {
        
        cbs_closeLogger(); // Close any files that were opened
        pthread_mutex_unlock(&log_mutex);
        return false;
    }
    
    // Generate instance ID for this logging session
    generate_instance_id();
    
    // Clean up old logs based on retention policy
    delete_old_logs(log_directory, MAX_LOG_RETENTION_DAYS);
    
    // Log initialization success
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "CBS Logger initialized. Instance ID: %s", instance_id);
    fprintf(application_log_file, "[%s] [%s] [%s] [%lu] %s\n", 
            get_timestamp_with_ms(), "INFO", get_operation_id(), ++sequence_number, buffer);
    
    pthread_mutex_unlock(&log_mutex);
    return true;
}

/**
 * Clean up and close logger resources
 */
void cbs_closeLogger(void) {
    pthread_mutex_lock(&log_mutex);
    
    // Close all log files if they're open
    if (application_log_file) {
        fclose(application_log_file);
        application_log_file = NULL;
    }
    
    if (transaction_log_file) {
        fclose(transaction_log_file);
        transaction_log_file = NULL;
    }
    
    if (security_log_file) {
        fclose(security_log_file);
        security_log_file = NULL;
    }
    
    if (audit_log_file) {
        fclose(audit_log_file);
        audit_log_file = NULL;
    }
    
    if (error_log_file) {
        fclose(error_log_file);
        error_log_file = NULL;
    }
    
    if (debug_log_file) {
        fclose(debug_log_file);
        debug_log_file = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Set the global minimum log level
 * @param level Minimum log level to record
 */
void cbs_setLogLevel(CBSLogLevel level) {
    pthread_mutex_lock(&log_mutex);
    current_log_level = level;
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Get the current global minimum log level
 * @return Current log level
 */
CBSLogLevel cbs_getLogLevel(void) {
    return current_log_level;
}

/**
 * Write a log message with DEBUG level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeDebugLog(const char *format, ...) {
    if (current_log_level > CBS_LOG_LEVEL_DEBUG)
        return;
        
    va_list args;
    va_start(args, format);
    internal_write_log(debug_log_file, CBS_LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

/**
 * Write a log message with INFO level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeInfoLog(const char *format, ...) {
    if (current_log_level > CBS_LOG_LEVEL_INFO)
        return;
        
    va_list args;
    va_start(args, format);
    internal_write_log(application_log_file, CBS_LOG_LEVEL_INFO, format, args);
    va_end(args);
}

/**
 * Write a log message with WARNING level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeWarningLog(const char *format, ...) {
    if (current_log_level > CBS_LOG_LEVEL_WARNING)
        return;
        
    va_list args;
    va_start(args, format);
    internal_write_log(application_log_file, CBS_LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

/**
 * Write a log message with ERROR level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeErrorLog(const char *format, ...) {
    if (current_log_level > CBS_LOG_LEVEL_ERROR)
        return;
        
    va_list args;
    va_start(args, format);
    internal_write_log(error_log_file, CBS_LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

/**
 * Write a log message with CRITICAL level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeCriticalLog(const char *format, ...) {
    if (current_log_level > CBS_LOG_LEVEL_CRITICAL)
        return;
        
    va_list args;
    va_start(args, format);
    internal_write_log(error_log_file, CBS_LOG_LEVEL_CRITICAL, format, args);
    va_end(args);
}

/**
 * Write a log message with SECURITY level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void cbs_writeSecurityLog(const char *format, ...) {
    va_list args;
    va_start(args, format);
    internal_write_log(security_log_file, CBS_LOG_LEVEL_SECURITY, format, args);
    va_end(args);
}

/**
 * Log a CBS transaction with full banking system details
 * @param transaction_id Unique transaction identifier
 * @param user_id User or system that initiated the transaction
 * @param card_number Card number (masked for security)
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
    const char *details) {
    
    if (!transaction_log_file) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    // Rotate log if needed
    rotate_log_if_needed(&transaction_log_file, "transactions.log");
    
    // Current timestamp with milliseconds
    char *time_str = get_timestamp_with_ms();
    
    // Mask sensitive data
    char masked_card[32] = {0};
    if (card_number) {
        size_t len = strlen(card_number);
        if (len >= 8) {
            // Keep first 6 and last 4 digits visible (PCI-DSS standard)
            size_t i;
            for (i = 0; i < 6 && i < len; i++) {
                masked_card[i] = card_number[i];
            }
            for (; i < len - 4; i++) {
                masked_card[i] = 'X';
            }
            for (i = len - 4; i < len; i++) {
                masked_card[i] = card_number[i];
            }
            masked_card[len] = '\0';
        } else if (len > 0) {
            // For shorter numbers, mask all but last 4 digits
            size_t i;
            for (i = 0; i < len - 4; i++) {
                masked_card[i] = 'X';
            }
            for (; i < len; i++) {
                masked_card[i] = card_number[i];
            }
            masked_card[len] = '\0';
        } else {
            strcpy(masked_card, "N/A");
        }
    } else {
        strcpy(masked_card, "N/A");
    }
    
    // Get operation ID
    char *op_id = get_operation_id();
    
    // Build the transaction log entry - ISO 8583 inspired format
    fprintf(transaction_log_file, 
            "%s|%s|%lu|%s|%s|%s|%s|%s|%.2f|%.2f|%.2f|%s|%s\n",
            time_str,
            op_id,
            ++sequence_number,
            transaction_id ? transaction_id : "N/A",
            user_id ? user_id : "SYSTEM",
            masked_card,
            account_id ? account_id : "N/A",
            transaction_type ? transaction_type : "UNKNOWN",
            amount,
            prev_balance,
            new_balance,
            status ? status : "UNKNOWN",
            details ? details : "");
    
    // Also log to console if enabled
    if (console_output_enabled) {
        printf("[%s] [TRANSACTION] %s %s: %s %.2f (%.2f → %.2f) %s\n",
               time_str,
               masked_card,
               account_id ? account_id : "N/A",
               transaction_type ? transaction_type : "UNKNOWN",
               amount,
               prev_balance,
               new_balance,
               status ? status : "UNKNOWN");
    }
    
    // Flush to ensure the log is written immediately
    fflush(transaction_log_file);
    
    free(time_str);
    free(op_id);
    
    pthread_mutex_unlock(&log_mutex);
}

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
    const char *details) {
    
    if (!audit_log_file) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    // Rotate log if needed
    rotate_log_if_needed(&audit_log_file, "audit.log");
    
    // Current timestamp with milliseconds
    char *time_str = get_timestamp_with_ms();
    
    // Get operation ID
    char *op_id = get_operation_id();
    
    // Build the audit log entry
    fprintf(audit_log_file, 
            "%s|%s|%lu|%s|%s|%s|%s|%s|%s|%s\n",
            time_str,
            op_id,
            ++sequence_number,
            user_id ? user_id : "SYSTEM",
            action_type ? action_type : "UNKNOWN",
            entity_type ? entity_type : "UNKNOWN",
            entity_id ? entity_id : "N/A",
            before_state ? before_state : "N/A",
            after_state ? after_state : "N/A",
            details ? details : "");
    
    // Flush to ensure the log is written immediately
    fflush(audit_log_file);
    
    free(time_str);
    free(op_id);
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Enable or disable console output for logs
 * @param enable true to enable console output, false to disable
 */
void cbs_enableConsoleOutput(bool enable) {
    pthread_mutex_lock(&log_mutex);
    console_output_enabled = enable;
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Flush all buffered logs to disk
 */
void cbs_flushLogs(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (application_log_file) fflush(application_log_file);
    if (transaction_log_file) fflush(transaction_log_file);
    if (security_log_file) fflush(security_log_file);
    if (audit_log_file) fflush(audit_log_file);
    if (error_log_file) fflush(error_log_file);
    if (debug_log_file) fflush(debug_log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Set the operation ID for the current thread
 * @param operation_id The operation ID to set
 */
void cbs_setOperationId(const char *operation_id) {
    pthread_mutex_lock(&log_mutex);
    
    // Store in thread-local storage or thread-specific data
    // This is a simplified implementation - in production code,
    // use pthread_setspecific or similar thread-local storage mechanisms
    
    static __thread char current_operation_id[64] = {0};
    if (operation_id) {
        strncpy(current_operation_id, operation_id, sizeof(current_operation_id) - 1);
        current_operation_id[sizeof(current_operation_id) - 1] = '\0';
    } else {
        current_operation_id[0] = '\0';
    }
    
    pthread_mutex_unlock(&log_mutex);
}

/* ==================== INTERNAL FUNCTIONS ==================== */

/**
 * Rotate log file if it exceeds maximum size
 * @param log_file Pointer to log file handle
 * @param filename Base filename for the log
 */
static void rotate_log_if_needed(FILE** log_file, const char* filename) {
    if (!*log_file)
        return;
        
    // Check file size
    long position = ftell(*log_file);
    
    // If file size is too large, close it and open a new one with timestamp in name
    if (position > MAX_LOG_SIZE_DEFAULT) {
        fclose(*log_file);
        
        char timestamp[20];
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_now);
        
        char old_path[512];
        char new_path[512];
        snprintf(old_path, sizeof(old_path), "%s/%s", log_directory, filename);
        snprintf(new_path, sizeof(new_path), "%s/%s.%s", log_directory, filename, timestamp);
        
        // Rename the old file
        rename(old_path, new_path);
        
        // Open a new file
        *log_file = fopen(old_path, "a");
    }
}

/**
 * Get the current time as a formatted string
 * @param buffer Buffer to store the timestamp
 * @param size Size of the buffer
 */
static void get_current_time_str(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_now);
}

/**
 * Get string representation of log level
 * @param level Log level enumeration value
 * @return String representation of the log level
 */
static const char* get_log_level_str(CBSLogLevel level) {
    switch (level) {
        case CBS_LOG_LEVEL_DEBUG:    return "DEBUG";
        case CBS_LOG_LEVEL_INFO:     return "INFO";
        case CBS_LOG_LEVEL_WARNING:  return "WARNING";
        case CBS_LOG_LEVEL_ERROR:    return "ERROR";
        case CBS_LOG_LEVEL_CRITICAL: return "CRITICAL";
        case CBS_LOG_LEVEL_SECURITY: return "SECURITY";
        default:                     return "UNKNOWN";
    }
}

/**
 * Format a log message with level and timestamp
 * @param level The log level
 * @param format Format string
 * @param args Variable argument list
 * @return Dynamically allocated formatted message (must be freed by caller)
 */
static char* format_log_message(CBSLogLevel level, const char* format, va_list args) {
    char* buffer = (char*)malloc(MAX_LOG_MESSAGE_LENGTH);
    if (!buffer) return NULL;
    
    char message[MAX_LOG_MESSAGE_LENGTH];
    vsnprintf(message, MAX_LOG_MESSAGE_LENGTH, format, args);
    
    char timestamp[32];
    get_current_time_str(timestamp, sizeof(timestamp));
    
    char* op_id = get_operation_id();
    
    snprintf(buffer, MAX_LOG_MESSAGE_LENGTH, "[%s] [%s] [%s] [%lu] %s", 
             timestamp, get_log_level_str(level), op_id, ++sequence_number, message);
    
    free(op_id);
    
    return buffer;
}

/**
 * Internal function to write a log message to the specified file
 * @param log_file The file to write to
 * @param level The log level
 * @param format Format string
 * @param args Variable argument list
 */
static void internal_write_log(FILE* log_file, CBSLogLevel level, const char* format, va_list args) {
    if (!log_file)
        return;
    
    pthread_mutex_lock(&log_mutex);
    
    char* log_message = format_log_message(level, format, args);
    if (log_message) {
        fprintf(log_file, "%s\n", log_message);
        
        if (console_output_enabled) {
            printf("%s\n", log_message);
        }
        
        fflush(log_file);
        free(log_message);
    }
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Mask PII data in a text buffer based on field name
 * @param text Buffer containing the text to mask
 * @param field_name Name of the field to identify PII content
 */
static void mask_pii_data(char* text, const char* field_name) {
    if (!text || !field_name)
        return;
    
    // Fields that should be masked
    const char* pii_fields[] = {
        "card", "cvv", "pin", "password", "ssn", "social", "dob", "birthdate",
        "email", "phone", "address", NULL
    };
    
    // Check if this field should be masked
    bool should_mask = false;
    for (int i = 0; pii_fields[i]; i++) {
        if (strcasestr(field_name, pii_fields[i])) {
            should_mask = true;
            break;
        }
    }
    
    if (should_mask) {
        size_t len = strlen(text);
        
        // Special handling for different PII types
        if (strcasestr(field_name, "card")) {
            // Card numbers: keep first 6 and last 4 digits (if long enough)
            if (len > 10) {
                for (size_t i = 6; i < len - 4; i++) {
                    if (isdigit((unsigned char)text[i])) {
                        text[i] = 'X';
                    }
                }
            } else {
                // Shorter strings - mask everything
                for (size_t i = 0; i < len; i++) {
                    if (isdigit((unsigned char)text[i])) {
                        text[i] = 'X';
                    }
                }
            }
        } else if (strcasestr(field_name, "cvv") || strcasestr(field_name, "pin") || 
                   strcasestr(field_name, "password")) {
            // Complete masking for CVV, PIN, passwords
            for (size_t i = 0; i < len; i++) {
                text[i] = '*';
            }
        } else if (strcasestr(field_name, "email")) {
            // Email: keep domain part, mask local part
            char* at_sign = strchr(text, '@');
            if (at_sign) {
                size_t local_part_len = at_sign - text;
                // Keep first character of local part if available
                if (local_part_len > 1) {
                    for (size_t i = 1; i < local_part_len; i++) {
                        text[i] = 'x';
                    }
                }
            }
        } else {
            // Default masking for other PII: show first and last character
            if (len > 2) {
                for (size_t i = 1; i < len - 1; i++) {
                    text[i] = '*';
                }
            }
        }
    }
}

/**
 * Ensure that a directory exists, creating it if necessary
 * @param dir_path Path to the directory
 * @return true if directory exists or was created, false otherwise
 */
static bool ensure_directory_exists(const char* dir_path) {
    struct stat st;
    
    // Check if directory exists
    if (stat(dir_path, &st) == 0) {
        // Exists, check if it's a directory
        return S_ISDIR(st.st_mode);
    }
    
    // Directory doesn't exist, try to create it
    #ifdef _WIN32
    int result = mkdir(dir_path);
    #else
    int result = mkdir(dir_path, 0755);
    #endif
    
    return (result == 0);
}

/**
 * Generate a unique instance ID for this logging session
 */
static void generate_instance_id(void) {
    // UUID generation (simplified version)
    // In a real implementation, use a proper UUID library
    
    srand((unsigned int)time(NULL));
    
    // Format: 8-4-4-4-12 hexadecimal digits
    sprintf(instance_id, 
            "%08x-%04x-%04x-%04x-%04x%08x",
            rand() & 0xffffffff,
            rand() & 0xffff,
            rand() & 0xffff,
            rand() & 0xffff,
            rand() & 0xffff,
            rand() & 0xffffffff);
}

/**
 * Get current timestamp with millisecond precision
 * @return Dynamically allocated timestamp string (must be freed by caller)
 */
static char* get_timestamp_with_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    struct tm* tm_now = localtime(&ts.tv_sec);
    
    char* buffer = (char*)malloc(32);
    if (!buffer) return NULL;
    
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", tm_now);
    sprintf(buffer + strlen(buffer), ".%03ld", ts.tv_nsec / 1000000);
    
    return buffer;
}

/**
 * Delete log files older than the specified retention period
 * @param log_dir Directory containing log files
 * @param retention_days Number of days to retain logs
 */
static void delete_old_logs(const char* log_dir, int retention_days) {
    // This is a simplified implementation
    // A real implementation would scan the directory and check file dates
    
    // Get the cutoff time
    time_t now = time(NULL);
    time_t cutoff = now - (retention_days * 24 * 60 * 60);
    
    // Log the cleanup initiation
    char message[256];
    sprintf(message, "Log cleanup initiated. Removing files older than %d days.", retention_days);
    
    if (application_log_file) {
        fprintf(application_log_file, "[%s] [INFO] [%s] [%lu] %s\n",
                get_timestamp_with_ms(), get_operation_id(), ++sequence_number, message);
    }
    
    // In a real implementation, iterate through files and remove old ones
}

/**
 * Get the operation ID for the current request context
 * @return Dynamically allocated operation ID string (must be freed by caller)
 */
static char* get_operation_id(void) {
    // In a real implementation, this would retrieve a thread-local operation ID
    // For now, we'll just return a copy of the instance ID
    
    char* op_id = (char*)malloc(37);
    if (!op_id) return NULL;
    
    strcpy(op_id, instance_id);
    return op_id;
}
