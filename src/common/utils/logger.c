#include "../../../include/common/utils/logger.h"
#include "../../../include/common/utils/path_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Ensure log directories exist
static void ensureLogDirectoriesExist() {
    mkdir("logs", 0755);  // Create logs directory if it doesn't exist
}

// Internal function to write log entry
static void writeLog(const char* logPath, const char* logType, const char* message) {
    ensureLogDirectoriesExist();
    
    FILE* logFile = fopen(logPath, "a");
    if (!logFile) {
        fprintf(stderr, "ERROR: Could not open log file: %s (%s)\n", 
                logPath, strerror(errno));
        return;
    }
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(logFile, "[%s] [%s] %s\n", timestamp, logType, message);
    fclose(logFile);
}

// Write to error log
void writeErrorLog(const char* message) {
    writeLog("logs/error.log", "ERROR", message);
}

// Compatibility with write_error_log
void write_error_log(const char* message) {
    writeErrorLog(message);
}

// Write to info log
void writeInfoLog(const char* message) {
    writeLog("logs/info.log", "INFO", message);
}

// Compatibility with write_info_log
void write_info_log(const char* message) {
    writeInfoLog(message);
}

// Write to audit log
void writeAuditLog(const char* category, const char* message) {
    ensureLogDirectoriesExist();
    
    // Open the audit log file to check if it exists and has content
    FILE* logFile = fopen("logs/audit.log", "r");
    int fileExists = (logFile != NULL);
    int isEmpty = 1;
    
    if (fileExists) {
        // Check if the file is empty
        fseek(logFile, 0, SEEK_END);
        isEmpty = (ftell(logFile) == 0);
        fclose(logFile);
    }
    
    // Open the file for appending
    logFile = fopen("logs/audit.log", "a");
    if (!logFile) {
        fprintf(stderr, "ERROR: Could not open audit log file (%s)\n", strerror(errno));
        return;
    }
    
    // Write headers if the file doesn't exist or is empty
    if (!fileExists || isEmpty) {
        fprintf(logFile, "Log ID  | User ID  | Action Type | Table Affected | Action Details                             | Timestamp\n");
        fprintf(logFile, "---------|----------|------------|---------------|------------------------------------------|---------------------\n");
    }
    
    // Generate a log ID (L5xxxx format)
    static int logCounter = 1; // This will reset when the program restarts
    
    // Try to find the last ID in the file to keep continuity
    if (logCounter == 1) {
        FILE* readFile = fopen("logs/audit.log", "r");
        if (readFile) {
            char line[1024];
            char lastLogId[10] = "L50000"; // Default starting point
            
            // Skip the header lines
            fgets(line, sizeof(line), readFile);
            fgets(line, sizeof(line), readFile);
            
            // Read through file to find last log ID
            while (fgets(line, sizeof(line), readFile)) {
                if (strlen(line) > 7 && line[0] == 'L' && line[1] == '5') {
                    // Extract the ID portion (assuming format is L5xxxx)
                    strncpy(lastLogId, line, 6);
                    lastLogId[6] = '\0';
                }
            }
            
            // Extract the numeric part and increment
            if (strlen(lastLogId) >= 6) {
                int lastNum = atoi(lastLogId + 2); // Skip "L5" prefix
                if (lastNum > 0) {
                    logCounter = lastNum + 1;
                }
            }
            
            fclose(readFile);
        }
    }
    
    char logId[10];
    sprintf(logId, "L5%04d", logCounter++);
    
    // Get current timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Extract user ID and table affected from the category
    // Assuming category may contain user info or be an action category
    char userId[20] = "System";    // Default if not specified
    char tableAffected[20] = "System";  // Default table
    
    // If category contains user info like "USER:C10001"
    if (strstr(category, "USER:") == category) {
        strncpy(userId, category + 5, sizeof(userId) - 1);
        userId[sizeof(userId) - 1] = '\0';
    } else if (strstr(category, "ADMIN:") == category) {
        strncpy(userId, category + 6, sizeof(userId) - 1);
        userId[sizeof(userId) - 1] = '\0';
        strcpy(tableAffected, "Admin");
    } else if (strcmp(category, "LOGIN") == 0 || strcmp(category, "LOGOUT") == 0) {
        strcpy(tableAffected, "Account");
    } else if (strcmp(category, "TRANSACTION") == 0) {
        strcpy(tableAffected, "Account");
    } else if (strcmp(category, "RECEIPT") == 0) {
        strcpy(tableAffected, "Transaction");
    } else if (strcmp(category, "SYSTEM") == 0) {
        strcpy(tableAffected, "System");
    }
    
    // Format the message to fit in the tabular structure
    // Trim or truncate the message if needed to fit in the column
    char formattedMessage[41]; // 40 chars + null terminator
    strncpy(formattedMessage, message, 40);
    formattedMessage[40] = '\0';
    
    // Write the formatted log entry to the file
    fprintf(logFile, "%-8s | %-8s | %-10s | %-14s | %-40s | %s\n", 
            logId, userId, category, tableAffected, formattedMessage, timestamp);
    
    fclose(logFile);
}

// Compatibility with write_audit_log
void write_audit_log(const char* category, const char* message) {
    writeAuditLog(category, message);
}

// Write to dedicated transaction log
void writeDetailedTransactionLog(const char* transactionType, const char* details) {
    FILE* transLog = fopen(TRANSACTIONS_LOG, "a");
    if (transLog) {
        // ...existing code...
    }
}

// Write to withdrawals log
void writeWithdrawalLog(int cardNumber, float amount) {
    char message[256];
    snprintf(message, sizeof(message), "Card: %d, Amount: %.2f", 
             cardNumber, amount);
    
    writeLog("logs/withdrawals.log", "WITHDRAWAL", message);
}