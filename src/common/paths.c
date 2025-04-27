#include "paths.h"
#include "../utils/memory_utils.h"
#include "../common/error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Global testing mode flag
static int testMode = 0;

// Set the testing mode flag
void setTestingMode(int isTest) {
    testMode = isTest;
}

// Get the current testing mode
int isTestingMode() {
    return testMode;
}

// Get file paths based on testing mode
const char* getCardFilePath() {
    return testMode ? TEST_CARD_FILE : PROD_CARD_FILE;
}

const char* getCustomerFilePath() {
    return testMode ? TEST_CUSTOMER_FILE : PROD_CUSTOMER_FILE;
}

const char* getAccountingFilePath() {
    return testMode ? TEST_ACCOUNTING_FILE : PROD_ACCOUNTING_FILE;
}

const char* getVirtualWalletFilePath() {
    return testMode ? TEST_VIRTUAL_WALLET_FILE : PROD_VIRTUAL_WALLET_FILE;
}

const char* getAdminCredentialsFilePath() {
    return testMode ? TEST_ADMIN_CREDENTIALS_FILE : PROD_ADMIN_CREDENTIALS_FILE;
}

const char* getSystemConfigFilePath() {
    return testMode ? TEST_SYSTEM_CONFIG_FILE : PROD_SYSTEM_CONFIG_FILE;
}

const char* getSecurityLogsFilePath() {
    return testMode ? TEST_SECURITY_LOGS_FILE : PROD_SECURITY_LOGS_FILE;
}

const char* getPinAttemptsFilePath() {
    return testMode ? TEST_PIN_ATTEMPTS_FILE : PROD_PIN_ATTEMPTS_FILE;
}

const char* getCardLockoutFilePath() {
    return testMode ? TEST_CARD_LOCKOUT_FILE : PROD_CARD_LOCKOUT_FILE;
}

// Get log paths based on testing mode
const char* getAuditLogFilePath() {
    return testMode ? TEST_AUDIT_LOG_FILE : PROD_AUDIT_LOG_FILE;
}

const char* getErrorLogFilePath() {
    return testMode ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE;
}

const char* getTransactionsLogFilePath() {
    return testMode ? TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
}

const char* getWithdrawalsLogFilePath() {
    return testMode ? TEST_WITHDRAWALS_LOG_FILE : PROD_WITHDRAWALS_LOG_FILE;
}

// Create a temporary file path
char* createTempFilePath(const char* baseFilePath) {
    size_t len = strlen(baseFilePath);
    char* tempPath = (char*)MALLOC(len + 5, "Temporary file path");
    
    if (!tempPath) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for temporary file path");
        return NULL;
    }
    
    strcpy(tempPath, baseFilePath);
    strcat(tempPath, ".tmp");
    
    return tempPath;
}

// Join path components safely
char* joinPaths(const char* dir, const char* filename) {
    if (!dir || !filename) {
        SET_ERROR(ERR_INVALID_INPUT, "Invalid directory or filename for path joining");
        return NULL;
    }
    
    size_t dirLen = strlen(dir);
    size_t fileLen = strlen(filename);
    size_t needsSep = (dirLen > 0 && dir[dirLen - 1] != '/') ? 1 : 0;
    
    char* fullPath = (char*)MALLOC(dirLen + fileLen + needsSep + 1, "Joined path");
    if (!fullPath) {
        SET_ERROR(ERR_MEMORY_ALLOCATION, "Failed to allocate memory for joined path");
        return NULL;
    }
    
    strcpy(fullPath, dir);
    
    if (needsSep) {
        strcat(fullPath, "/");
    }
    
    strcat(fullPath, filename);
    
    return fullPath;
}

// Cross-platform directory creation
int createDirectory(const char* path) {
#ifdef _WIN32
    // Windows
    int result = mkdir(path);
#else
    // Unix-like systems
    int result = mkdir(path, 0755);
#endif
    
    if (result != 0) {
        // If directory already exists, it's not an error
        if (errno == EEXIST) {
            return 1;
        }
        
        char errMsg[256];
        snprintf(errMsg, sizeof(errMsg), "Failed to create directory '%s'", path);
        SET_ERROR(ERR_FILE_ACCESS, errMsg);
        return 0;
    }
    
    return 1;
}

// Ensure directory exists
int ensureDirectoryExists(const char* dirPath) {
    return createDirectory(dirPath);
}

// Check if a file exists
static int fileExists(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Create an empty file if it doesn't exist
static int ensureFileExists(const char* filePath) {
    if (fileExists(filePath)) {
        return 1; // File already exists
    }
    
    FILE* file = fopen(filePath, "w");
    if (!file) {
        char errMsg[256];
        snprintf(errMsg, sizeof(errMsg), "Failed to create file '%s'", filePath);
        SET_ERROR(ERR_FILE_ACCESS, errMsg);
        return 0;
    }
    
    fclose(file);
    return 1;
}

// Initialize directories and files
int initializeDataFiles() {
    // Create required directories
    if (!ensureDirectoryExists(PROD_DATA_DIR)) return 0;
    if (!ensureDirectoryExists(PROD_LOG_DIR)) return 0;
    if (!ensureDirectoryExists(TEST_DATA_DIR)) return 0;
    if (!ensureDirectoryExists("data/temp")) return 0;
    
    // Ensure essential files exist
    const char* essentialFiles[] = {
        PROD_CARD_FILE,
        PROD_CUSTOMER_FILE,
        PROD_ACCOUNTING_FILE,
        PROD_VIRTUAL_WALLET_FILE,
        PROD_ADMIN_CREDENTIALS_FILE,
        PROD_SYSTEM_CONFIG_FILE,
        PROD_SECURITY_LOGS_FILE,
        TEST_CARD_FILE,
        TEST_CUSTOMER_FILE,
        TEST_ACCOUNTING_FILE,
        TEST_VIRTUAL_WALLET_FILE,
        TEST_ADMIN_CREDENTIALS_FILE,
        TEST_SYSTEM_CONFIG_FILE,
        TEST_SECURITY_LOGS_FILE
    };
    
    int fileCount = sizeof(essentialFiles) / sizeof(essentialFiles[0]);
    
    for (int i = 0; i < fileCount; i++) {
        if (!ensureFileExists(essentialFiles[i])) {
            return 0;
        }
    }
    
    return 1;
}