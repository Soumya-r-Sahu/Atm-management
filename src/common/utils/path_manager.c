#include "../../../include/common/utils/path_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Global testing mode flag
static int testingMode = 0;

// Set the testing mode
void setTestingMode(int isTestMode) {
    testingMode = isTestMode;
}

// Check if testing mode is enabled
int isTestingMode() {
    return testingMode;
}

// Ensure a directory exists
int ensureDirectoryExists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        // Directory doesn't exist, try to create it
        #ifdef _WIN32
        if (mkdir(path) != 0) {
            return 0; // Failed to create directory
        }
        #else
        if (mkdir(path, 0755) != 0) {
            return 0; // Failed to create directory
        }
        #endif
    }
    return 1; // Directory exists or was created successfully
}

// Get the path to the card file
const char* getCardFilePath() {
    return isTestingMode() ? "data/test/card.txt" : "data/card.txt";
}

// Get the path to the customer file
const char* getCustomerFilePath() {
    return isTestingMode() ? "data/test/customer.txt" : "data/customer.txt";
}

// Get the path to the accounting file
const char* getAccountingFilePath() {
    return isTestingMode() ? "data/test/accounting.txt" : "data/accounting.txt";
}

// Get the path to the admin credentials file
const char* getAdminCredentialsFilePath() {
    return isTestingMode() ? "data/test/admin_credentials.txt" : "data/admin_credentials.txt";
}

// Get the path to the system config file
const char* getSystemConfigFilePath() {
    return isTestingMode() ? "data/test/system_config.txt" : "data/system_config.txt";
}

// Get the path to the transaction log file
const char* getTransactionLogFilePath() {
    return isTestingMode() ? "logs/test/transactions.log" : "logs/transactions.log";
}

// Get the path to the error log file
const char* getErrorLogFilePath() {
    return isTestingMode() ? "logs/test/error.log" : "logs/error.log";
}

// Get the path to the audit log file
const char* getAuditLogFilePath() {
    return isTestingMode() ? "logs/test/audit.log" : "logs/audit.log";
}

// Get the path to the withdrawal log file
const char* getWithdrawalLogFilePath() {
    return isTestingMode() ? "logs/test/withdrawals.log" : "logs/withdrawals.log";
}