#include "paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>     // For _mkdir on Windows
#include <io.h>         // For _access on Windows
#include <sys/types.h>
#include <sys/stat.h>   // For _stat on Windows
#define MKDIR(path) _mkdir(path)
#define S_IFDIR _S_IFDIR
#define stat _stat
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO)
#endif

#include <errno.h>

// Static variable to track testing mode
static bool testMode = false;

// Check if the application is in testing mode
bool isTestingMode() {
    return testMode;
}

// Set the application to use test or production data
void setTestingMode(bool testing) {
    testMode = testing;
}

// Create directory if it doesn't exist
static bool createDirectoryIfNotExists(const char* path) {
#ifdef _WIN32
    // For Windows
    struct stat info;
    if (stat(path, &info) != 0) {
        // Path doesn't exist, create it
        int result = MKDIR(path);
        return (result == 0);
    } else if (info.st_mode & S_IFDIR) {
        // Path exists and is a directory
        return true;
    }
    // Path exists but is not a directory
    return false;
#else
    // For Unix-like systems
    struct stat info;
    if (stat(path, &info) != 0) {
        // Path doesn't exist, create it
        int result = MKDIR(path);
        return (result == 0);
    } else if (S_ISDIR(info.st_mode)) {
        // Path exists and is a directory
        return true;
    }
    // Path exists but is not a directory
    return false;
#endif
}

// Create an empty file if it doesn't exist
static bool createFileIfNotExists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        // File doesn't exist, create it
        file = fopen(path, "w");
        if (file == NULL) {
            return false;
        }
        fclose(file);
        return true;
    }
    // File already exists
    fclose(file);
    return true;
}

// Create template credentials file
static void createTemplateCredentialsFile(const char* path) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return;
    }
    
    fprintf(file, "Account Holder Name | Phone Number  | Card Number | PIN  | Status\n");
    fprintf(file, "--------------------|---------------|-------------|------|--------\n");
    fprintf(file, "Soumya               | 9876543210    | 100041      | 1467 | Active\n");
    fprintf(file, "Rahul                | 8765432109    | 106334      | 9500 | Active\n");
    
    fclose(file);
}

// Create template accounting file
static void createTemplateAccountingFile(const char* path) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return;
    }
    
    fprintf(file, "Card Number | Balance\n");
    fprintf(file, "------------|--------\n");
    fprintf(file, "100041       | 500.00\n");
    fprintf(file, "106334       | 15545.00\n");
    
    fclose(file);
}

// Create template admin credentials file
static void createTemplateAdminCredentialsFile(const char* path) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return;
    }
    
    fprintf(file, "admin admin123\n");
    
    fclose(file);
}

// Initialize all necessary data files and directories
bool initializeDataFiles() {
    bool success = true;
    
    // Create directories
    success &= createDirectoryIfNotExists(PROD_DATA_DIR);
    success &= createDirectoryIfNotExists(TEST_DATA_DIR);
    
    // Create logs directory
    char logsDir[100];
    strcpy(logsDir, PROD_DATA_DIR);
    strcat(logsDir, "/../logs");
    success &= createDirectoryIfNotExists(logsDir);
    
    // Create temp directory
    char tempDir[100];
    strcpy(tempDir, PROD_DATA_DIR);
    strcat(tempDir, "/temp");
    success &= createDirectoryIfNotExists(tempDir);
    
    // Create production data files if they don't exist
    if (!createFileIfNotExists(PROD_CREDENTIALS_FILE)) {
        createTemplateCredentialsFile(PROD_CREDENTIALS_FILE);
    }
    
    if (!createFileIfNotExists(PROD_ACCOUNTING_FILE)) {
        createTemplateAccountingFile(PROD_ACCOUNTING_FILE);
    }
    
    createFileIfNotExists(PROD_ADMIN_CRED_FILE);
    createFileIfNotExists(PROD_AUDIT_LOG_FILE);
    createFileIfNotExists(PROD_STATUS_FILE);
    createFileIfNotExists(PROD_LANGUAGES_FILE);
    createFileIfNotExists(PROD_TRANSACTIONS_LOG_FILE);
    createFileIfNotExists(PROD_ERROR_LOG_FILE);
    
    // Create test data files if they don't exist
    if (!createFileIfNotExists(TEST_CREDENTIALS_FILE)) {
        createTemplateCredentialsFile(TEST_CREDENTIALS_FILE);
    }
    
    if (!createFileIfNotExists(TEST_ACCOUNTING_FILE)) {
        createTemplateAccountingFile(TEST_ACCOUNTING_FILE);
    }
    
    if (!createFileIfNotExists(TEST_ADMIN_CRED_FILE)) {
        createTemplateAdminCredentialsFile(TEST_ADMIN_CRED_FILE);
    }
    
    createFileIfNotExists(TEST_AUDIT_LOG_FILE);
    createFileIfNotExists(TEST_STATUS_FILE);
    createFileIfNotExists(TEST_LANGUAGES_FILE);
    createFileIfNotExists(TEST_TRANSACTIONS_LOG_FILE);
    createFileIfNotExists(TEST_ERROR_LOG_FILE);
    
    return success;
}