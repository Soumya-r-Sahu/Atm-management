/**
 * @file logger.c
 * @brief Logging utility implementation for the ATM Management System
 * @version 1.0
 * @date May 3, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "common/utils/logger.h"

// Maximum length of log message
#define MAX_LOG_MESSAGE_LENGTH 4096

// Log file handles
static FILE* debug_log_file = NULL;
static FILE* info_log_file = NULL;
static FILE* warning_log_file = NULL;
static FILE* error_log_file = NULL;
static FILE* security_log_file = NULL;
static FILE* transaction_log_file = NULL;

// Log directory
static char log_directory[256] = "logs";

// Current log level
static LogLevel current_log_level = LOG_LEVEL_INFO;

// Console output flag
static bool console_output_enabled = true;

// Thread safety mutex
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Log rotation settings
static const long MAX_LOG_SIZE = 10 * 1024 * 1024; // 10 MB

// Forward declarations of internal functions
static void rotate_log_if_needed(FILE** log_file, const char* filename);
static void get_current_time_str(char* buffer, size_t size);
static const char* get_log_level_str(LogLevel level);
static void format_log_message(char* buffer, size_t size, LogLevel level, const char* format, va_list args);
static void internal_write_log(FILE* log_file, LogLevel level, const char* format, va_list args);
static void mask_card_number(const char* card_number, char* masked, size_t size);
static bool ensure_directory_exists(const char* dir_path);

/**
 * Initialize the logger system
 * @param log_dir Directory to store log files
 * @return true if initialization successful, false otherwise
 */
bool initializeLogger(const char* log_dir) {
    pthread_mutex_lock(&log_mutex);
    
    // Store log directory
    if (log_dir) {
        strncpy(log_directory, log_dir, sizeof(log_directory) - 1);
        log_directory[sizeof(log_directory) - 1] = '\0';
    }
    
    // Ensure log directory exists
    if (!ensure_directory_exists(log_directory)) {
        fprintf(stderr, "Failed to create log directory: %s\n", log_directory);
        pthread_mutex_unlock(&log_mutex);
        return false;
    }
    
    // Construct file paths
    char debug_path[512], info_path[512], warning_path[512], error_path[512], security_path[512], transaction_path[512];
    
    snprintf(debug_path, sizeof(debug_path), "%s/debug.log", log_directory);
    snprintf(info_path, sizeof(info_path), "%s/info.log", log_directory);
    snprintf(warning_path, sizeof(warning_path), "%s/warning.log", log_directory);
    snprintf(error_path, sizeof(error_path), "%s/error.log", log_directory);
    snprintf(security_path, sizeof(security_path), "%s/security.log", log_directory);
    snprintf(transaction_path, sizeof(transaction_path), "%s/transactions.log", log_directory);
    
    // Open log files
    debug_log_file = fopen(debug_path, "a");
    info_log_file = fopen(info_path, "a");
    warning_log_file = fopen(warning_path, "a");
    error_log_file = fopen(error_path, "a");
    security_log_file = fopen(security_path, "a");
    transaction_log_file = fopen(transaction_path, "a");
    
    // Check if all files were opened successfully
    bool success = (debug_log_file && info_log_file && warning_log_file && 
                   error_log_file && security_log_file && transaction_log_file);
    
    if (!success) {
        fprintf(stderr, "Failed to open one or more log files\n");
        closeLogger();
    } else {
        // Write initialization message
        writeInfoLog("Logger initialized. Log directory: %s", log_directory);
    }
    
    pthread_mutex_unlock(&log_mutex);
    return success;
}

/**
 * Clean up and close logger resources
 */
void closeLogger(void) {
    pthread_mutex_lock(&log_mutex);
    
    // Close log files if open
    if (debug_log_file) {
        fclose(debug_log_file);
        debug_log_file = NULL;
    }
    
    if (info_log_file) {
        fclose(info_log_file);
        info_log_file = NULL;
    }
    
    if (warning_log_file) {
        fclose(warning_log_file);
        warning_log_file = NULL;
    }
    
    if (error_log_file) {
        fclose(error_log_file);
        error_log_file = NULL;
    }
    
    if (security_log_file) {
        fclose(security_log_file);
        security_log_file = NULL;
    }
    
    if (transaction_log_file) {
        fclose(transaction_log_file);
        transaction_log_file = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Set the global minimum log level
 * @param level Minimum log level to record
 */
void setLogLevel(LogLevel level) {
    pthread_mutex_lock(&log_mutex);
    current_log_level = level;
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Get the current global minimum log level
 * @return Current log level
 */
LogLevel getLogLevel(void) {
    LogLevel level;
    
    pthread_mutex_lock(&log_mutex);
    level = current_log_level;
    pthread_mutex_unlock(&log_mutex);
    
    return level;
}

/**
 * Write a log message with the DEBUG level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeDebugLog(const char *format, ...) {
    va_list args;
    
    // Check if this log level is enabled
    if (getLogLevel() > LOG_LEVEL_DEBUG) {
        return;
    }
    
    va_start(args, format);
    internal_write_log(debug_log_file, LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

/**
 * Write a log message with the INFO level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeInfoLog(const char *format, ...) {
    va_list args;
    
    // Check if this log level is enabled
    if (getLogLevel() > LOG_LEVEL_INFO) {
        return;
    }
    
    va_start(args, format);
    internal_write_log(info_log_file, LOG_LEVEL_INFO, format, args);
    va_end(args);
}

/**
 * Write a log message with the WARNING level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeWarningLog(const char *format, ...) {
    va_list args;
    
    // Check if this log level is enabled
    if (getLogLevel() > LOG_LEVEL_WARNING) {
        return;
    }
    
    va_start(args, format);
    internal_write_log(warning_log_file, LOG_LEVEL_WARNING, format, args);
    va_end(args);
}

/**
 * Write a log message with the ERROR level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeErrorLog(const char *format, ...) {
    va_list args;
    
    // Check if this log level is enabled
    if (getLogLevel() > LOG_LEVEL_ERROR) {
        return;
    }
    
    va_start(args, format);
    internal_write_log(error_log_file, LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

/**
 * Write a log message with the CRITICAL level
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeCriticalLog(const char *format, ...) {
    va_list args;
    
    // Check if this log level is enabled
    if (getLogLevel() > LOG_LEVEL_CRITICAL) {
        return;
    }
    
    va_start(args, format);
    internal_write_log(error_log_file, LOG_LEVEL_CRITICAL, format, args);
    va_end(args);
}

/**
 * Write a log message to the security log
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void writeSecurityLog(const char *format, ...) {
    va_list args;
    
    va_start(args, format);
    internal_write_log(security_log_file, LOG_LEVEL_SECURITY, format, args);
    va_end(args);
}

/**
 * Write a transaction log
 * @param transaction_type Transaction type
 * @param card_number Card number (will be masked for security)
 * @param amount Transaction amount
 * @param status Transaction status
 * @param details Additional transaction details
 */
void writeTransactionLog(const char *transaction_type, 
                         const char *card_number, 
                         double amount,
                         const char *status,
                         const char *details) {
    if (!transaction_log_file) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    // Rotate log if needed
    rotate_log_if_needed(&transaction_log_file, "transactions.log");
    
    char time_str[32];
    get_current_time_str(time_str, sizeof(time_str));
    
    // Mask card number for security
    char masked_card[32];
    mask_card_number(card_number, masked_card, sizeof(masked_card));
    
    // Build the log entry
    fprintf(transaction_log_file, "[%s] [%s] Card %s: %s of $%.2f %s%s%s\n",
            time_str, 
            transaction_type,
            masked_card,
            transaction_type,
            amount,
            status ? status : "",
            details ? " - " : "",
            details ? details : "");
    
    // Also log to console if enabled
    if (console_output_enabled) {
        printf("[%s] [%s] Card %s: %s of $%.2f %s%s%s\n",
               time_str, 
               transaction_type,
               masked_card,
               transaction_type,
               amount,
               status ? status : "",
               details ? " - " : "",
               details ? details : "");
    }
    
    // Flush to ensure the log is written immediately
    fflush(transaction_log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Enable or disable console output for logs
 * @param enable true to enable console output, false to disable
 */
void enableConsoleOutput(bool enable) {
    pthread_mutex_lock(&log_mutex);
    console_output_enabled = enable;
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Flush all buffered logs to disk
 */
void flushLogs(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (debug_log_file) fflush(debug_log_file);
    if (info_log_file) fflush(info_log_file);
    if (warning_log_file) fflush(warning_log_file);
    if (error_log_file) fflush(error_log_file);
    if (security_log_file) fflush(security_log_file);
    if (transaction_log_file) fflush(transaction_log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

/************************* Internal Helper Functions *************************/

/**
 * Internal function to write a log entry
 * @param log_file File to write to
 * @param level Log level
 * @param format Format string
 * @param args Variable arguments list
 */
static void internal_write_log(FILE* log_file, LogLevel level, const char* format, va_list args) {
    if (!log_file) {
        // If the log file isn't available, try to initialize logger with default path
        if (!initializeLogger(NULL)) {
            return;
        }
        
        // Check again after initialization attempt
        if (!log_file) {
            return;
        }
    }
    
    pthread_mutex_lock(&log_mutex);
    
    // Rotate log if needed
    switch (level) {
        case LOG_LEVEL_DEBUG:
            rotate_log_if_needed(&debug_log_file, "debug.log");
            break;
        case LOG_LEVEL_INFO:
            rotate_log_if_needed(&info_log_file, "info.log");
            break;
        case LOG_LEVEL_WARNING:
            rotate_log_if_needed(&warning_log_file, "warning.log");
            break;
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_CRITICAL:
            rotate_log_if_needed(&error_log_file, "error.log");
            break;
        case LOG_LEVEL_SECURITY:
            rotate_log_if_needed(&security_log_file, "security.log");
            break;
    }
    
    // Format the log message
    char log_message[MAX_LOG_MESSAGE_LENGTH];
    format_log_message(log_message, sizeof(log_message), level, format, args);
    
    // Write to log file
    fputs(log_message, log_file);
    fputc('\n', log_file);
    
    // Also write to error log for critical messages
    if (level == LOG_LEVEL_CRITICAL && error_log_file != log_file) {
        fputs(log_message, error_log_file);
        fputc('\n', error_log_file);
    }
    
    // Also write to console if enabled
    if (console_output_enabled) {
        puts(log_message);
    }
    
    // Flush critical and security logs immediately
    if (level == LOG_LEVEL_CRITICAL || level == LOG_LEVEL_SECURITY) {
        fflush(log_file);
        if (level == LOG_LEVEL_CRITICAL) {
            fflush(error_log_file);
        }
    }
    
    pthread_mutex_unlock(&log_mutex);
}

/**
 * Rotate a log file if it grows beyond the maximum size
 * @param log_file Pointer to the log file handle
 * @param filename Base name of the log file
 */
static void rotate_log_if_needed(FILE** log_file, const char* filename) {
    if (!log_file || !*log_file) {
        return;
    }
    
    // Get current file size
    long file_size;
    fseek(*log_file, 0, SEEK_END);
    file_size = ftell(*log_file);
    
    // If file size exceeds limit, rotate the log
    if (file_size > MAX_LOG_SIZE) {
        char old_path[512], new_path[512];
        time_t now;
        struct tm *timeinfo;
        char timestamp[32];
        
        // Generate timestamp for the rotated log filename
        time(&now);
        timeinfo = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
        
        // Construct file paths
        snprintf(old_path, sizeof(old_path), "%s/%s", log_directory, filename);
        snprintf(new_path, sizeof(new_path), "%s/%s.%s", log_directory, filename, timestamp);
        
        // Close current file
        fclose(*log_file);
        *log_file = NULL;
        
        // Rename the file
        rename(old_path, new_path);
        
        // Open a new log file
        *log_file = fopen(old_path, "a");
        
        // Write a rotation message
        if (*log_file) {
            fprintf(*log_file, "[%s] Log rotated. Previous log saved as %s\n", 
                    timestamp, new_path);
        }
    }
}

/**
 * Get current time as a formatted string
 * @param buffer Buffer to store the time string
 * @param size Size of the buffer
 */
static void get_current_time_str(char* buffer, size_t size) {
    time_t now;
    struct tm *timeinfo;
    
    time(&now);
    timeinfo = localtime(&now);
    
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/**
 * Get string representation of log level
 * @param level Log level
 * @return String representation
 */
static const char* get_log_level_str(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARNING:
            return "WARNING";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_CRITICAL:
            return "CRITICAL";
        case LOG_LEVEL_SECURITY:
            return "SECURITY";
        default:
            return "UNKNOWN";
    }
}

/**
 * Format a log message with timestamp and level
 * @param buffer Buffer to store the formatted message
 * @param size Size of the buffer
 * @param level Log level
 * @param format Format string
 * @param args Variable arguments list
 */
static void format_log_message(char* buffer, size_t size, LogLevel level, const char* format, va_list args) {
    char time_str[32];
    char message[MAX_LOG_MESSAGE_LENGTH - 100]; // Leave room for timestamp and level
    
    // Get current time
    get_current_time_str(time_str, sizeof(time_str));
    
    // Format the message
    vsnprintf(message, sizeof(message), format, args);
    
    // Combine timestamp, level, and message
    snprintf(buffer, size, "[%s] [%s] %s", time_str, get_log_level_str(level), message);
}

/**
 * Mask a card number for security
 * @param card_number Original card number
 * @param masked Buffer to store masked card number
 * @param size Size of the buffer
 */
static void mask_card_number(const char* card_number, char* masked, size_t size) {
    if (!card_number || !masked || size == 0) {
        return;
    }
    
    size_t len = strlen(card_number);
    
    // If the card number is too short, just copy it
    if (len <= 4) {
        strncpy(masked, card_number, size - 1);
        masked[size - 1] = '\0';
        return;
    }
    
    // Mask all but the last 4 digits
    size_t prefix_len = len > 4 ? len - 4 : 0;
    size_t i;
    
    // Fill with asterisks for all but the last 4 digits
    for (i = 0; i < prefix_len && i < size - 5; i++) {
        masked[i] = '*';
    }
    
    // Copy the last 4 digits
    size_t j;
    for (j = 0; i < size - 1 && j < 4; i++, j++) {
        masked[i] = card_number[prefix_len + j];
    }
    
    masked[i] = '\0';
}

/**
 * Ensure a directory exists, creating it if necessary
 * @param dir_path Directory path
 * @return true if the directory exists or was created, false otherwise
 */
static bool ensure_directory_exists(const char* dir_path) {
    struct stat st;
    
    // Check if directory already exists
    if (stat(dir_path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    
    // Create the directory
    if (mkdir(dir_path, 0755) != 0) {
        return false;
    }
    
    return true;
}