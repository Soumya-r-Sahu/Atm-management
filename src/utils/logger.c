#include "logger.h"
#include "../common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Get current timestamp as a string
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Write to error log file
void writeErrorLog(const char *message) {
    FILE *file = fopen(ERROR_LOG_FILE, "a");
    if (file == NULL) {
        printf("Error: Could not open error log file.\n");
        return;
    }
    
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    fprintf(file, "[%s] ERROR: %s\n", timestamp, message);
    fclose(file);
}

// Write to audit log file
void writeAuditLog(const char *category, const char *message) {
    FILE *file = fopen(AUDIT_LOG_FILE, "a");
    if (file == NULL) {
        writeErrorLog("Could not open audit log file");
        return;
    }
    
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    fprintf(file, "[%s] %s: %s\n", timestamp, category, message);
    fclose(file);
}

// Write to transaction log file
void writeTransactionLog(int cardNumber, const char *transactionType, float amount, int success) {
    FILE *file = fopen(TRANSACTIONS_LOG_FILE, "a");
    if (file == NULL) {
        writeErrorLog("Could not open transaction log file");
        return;
    }
    
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    fprintf(file, "[%s] Card: %d - %s - Amount: %.2f - %s\n", 
           timestamp, cardNumber, transactionType, amount, 
           success ? "Success" : "Failed");
    
    fclose(file);
}

// Log a withdrawal for daily limit tracking
void logWithdrawalForLimit(int cardNumber, float amount, const char *date) {
    // The withdrawals log file is separate from the regular transaction log
    FILE *file = fopen(isTestingMode() ? 
                      TEST_DATA_DIR "/test_withdrawals.log" : 
                      PROD_DATA_DIR "/../logs/withdrawals.log", "a");
    
    if (file == NULL) {
        writeErrorLog("Could not open withdrawals log file");
        return;
    }
    
    fprintf(file, "%d|%s|%.2f\n", cardNumber, date, amount);
    fclose(file);
}

// Clear a log file (e.g., for resetting daily withdrawals)
void clearLogFile(const char *logFile) {
    FILE *file = fopen(logFile, "w");
    if (file == NULL) {
        char errorMsg[100];
        sprintf(errorMsg, "Could not clear log file: %s", logFile);
        writeErrorLog(errorMsg);
        return;
    }
    
    fclose(file);
}

// Write to info log file
void writeInfoLog(const char *message) {
    FILE *file = fopen(ERROR_LOG_FILE, "a");
    if (file == NULL) {
        printf("Error: Could not open log file.\n");
        return;
    }
    
    char timestamp[30];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    fprintf(file, "[%s] INFO: %s\n", timestamp, message);
    fclose(file);
}