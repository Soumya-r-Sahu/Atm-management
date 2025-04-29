#include "logger.h"
#include "../common/paths.h"
#include "../transaction/transaction_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Enhanced error logging with additional context
void writeExtendedErrorLog(const char *file, int line, const char *function, const char *message) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Format for detailed error logs
    char logEntry[512];
    sprintf(logEntry, "[%s] [ERROR] [%s:%d in %s] %s\n", 
            timestamp, file, line, function, message);
    
    // Write to error log file
    const char* logPath = isTestingMode() ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE;
    FILE *logFile = fopen(logPath, "a");
    
    if (logFile != NULL) {
        fprintf(logFile, "%s", logEntry);
        fclose(logFile);
    } else {
        // Fall back to stderr if log file cannot be opened
        fprintf(stderr, "Failed to write to error log. Error was: %s\n", logEntry);
    }
}

// Create a macro to simplify calling the extended error log
#define LOG_ERROR(message) writeExtendedErrorLog(__FILE__, __LINE__, __func__, message)

// Standard error logging (kept for backward compatibility)
void writeErrorLog(const char *message) {
    writeExtendedErrorLog(__FILE__, __LINE__, __func__, message);
}

// Write info log messages
void writeInfoLog(const char *message) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Format for info logs
    char logEntry[512];
    sprintf(logEntry, "[%s] [INFO] %s\n", timestamp, message);
    
    // Write to the same log file as errors but with INFO tag
    const char* logPath = isTestingMode() ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE;
    FILE *logFile = fopen(logPath, "a");
    
    if (logFile != NULL) {
        fprintf(logFile, "%s", logEntry);
        fclose(logFile);
    }
}

// Write audit log entries
void writeAuditLog(const char *category, const char *message) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Format for audit logs
    char logEntry[512];
    sprintf(logEntry, "[%s] [%s] %s\n", timestamp, category, message);
    
    const char* auditPath = isTestingMode() ? TEST_AUDIT_LOG_FILE : PROD_AUDIT_LOG_FILE;
    FILE *auditFile = fopen(auditPath, "a");
    
    if (auditFile != NULL) {
        fprintf(auditFile, "%s", logEntry);
        fclose(auditFile);
    } else {
        // If audit log cannot be opened, fall back to error log
        writeErrorLog("Failed to write to audit log");
    }
}

// Log transaction with success/failure status
void writeTransactionLog(int cardNumber, const char* transactionType, float amount, int success) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    // Format for transaction logs
    char logEntry[512];
    sprintf(logEntry, "[%s] Card: %d, Type: %s, Amount: $%.2f, Status: %s\n", 
            timestamp, cardNumber, transactionType, amount, success ? "Success" : "Failed");
    
    const char* transactionPath = isTestingMode() ? 
        TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
    FILE *transactionFile = fopen(transactionPath, "a");
    
    if (transactionFile != NULL) {
        fprintf(transactionFile, "%s", logEntry);
        fclose(transactionFile);
    } else {
        // If transaction log cannot be opened, fall back to error log
        writeErrorLog("Failed to write to transaction log");
    }
}

// Log withdrawal for daily limit tracking
void logWithdrawal(int cardNumber, float amount) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    char dateOnly[11];
    strftime(dateOnly, sizeof(dateOnly), "%Y-%m-%d", tm_now);
    
    // Format for withdrawal logs
    char logEntry[256];
    sprintf(logEntry, "%d,%s,%.2f,%s\n", cardNumber, dateOnly, amount, timestamp);
    
    const char* withdrawalPath = isTestingMode() ? 
        "testing/test_withdrawals.log" : "logs/withdrawals.log";
    FILE *withdrawalFile = fopen(withdrawalPath, "a");
    
    if (withdrawalFile != NULL) {
        fprintf(withdrawalFile, "%s", logEntry);
        fclose(withdrawalFile);
    } else {
        // If withdrawal log cannot be opened, fall back to error log
        writeErrorLog("Failed to write to withdrawal log");
    }
}