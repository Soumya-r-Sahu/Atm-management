#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

// Get current timestamp formatted as string
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Log user activities to logs/transactions.log
void logActivity(const char *activity) {
    FILE *file = fopen("logs/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file.\n");
        return;
    }

    char timestamp[100];
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    fprintf(file, "[%s] USER: %s\n", timestamp, activity);
    fclose(file);
}

// Log error messages to logs/error.log
void writeErrorLog(const char *errorMsg) {
    FILE *file = fopen("logs/error.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open error log file.\n");
        return;
    }

    char timestamp[100];
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    fprintf(file, "[%s] ERROR: %s\n", timestamp, errorMsg);
    fclose(file);
}

// Log transaction details to logs/transactions.log
void writeTransactionLog(const char *accountHolderName, const char *operation, const char *details) {
    FILE *file = fopen("logs/transactions.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open transactions log file.\n");
        return;
    }

    char timestamp[100];
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    fprintf(file, "[%s] User: %s | Operation: %s | Details: %s\n",
            timestamp, accountHolderName, operation, details);
    fclose(file);
}

// Log audit information to logs/audit.log
void writeAuditLog(const char *action, const char *details) {
    FILE *file = fopen("logs/audit.log", "a");
    if (file == NULL) {
        printf("Error: Unable to open audit log file.\n");
        return;
    }

    char timestamp[100];
    getCurrentTimestamp(timestamp, sizeof(timestamp));

    fprintf(file, "[%s] %s: %s\n", timestamp, action, details);
    fclose(file);
}

// Legacy logging functions (console-based)
static void log_message(const char *level, const char *fmt, va_list args) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] [%s] ", t->tm_hour, t->tm_min, t->tm_sec, level);
    vprintf(fmt, args);
    printf("\n");
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("INFO", fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("WARN", fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message("ERROR", fmt, args);
    va_end(args);
    
    // Also write to error log file
    char buffer[1024];
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    writeErrorLog(buffer);
}