#include "common/utils/path_manager.h"
#include "common/utils/logger.h"
#include "common/paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

// Global flag to track if we're in testing mode
static int g_is_test_mode = 0;

// Set testing mode
void setTestingMode(int isTestMode) {
    g_is_test_mode = isTestMode;
}

// Check if in testing mode
int isTestingMode() {
    return g_is_test_mode;
}

// Create a temporary file path
char* create_temp_file_path(const char* base_file_path) {
    char* temp_path = malloc(strlen(base_file_path) + 5); // +5 for ".tmp\0"
    if (!temp_path) {
        return NULL;
    }
    strcpy(temp_path, base_file_path);
    strcat(temp_path, ".tmp");
    return temp_path;
}

// Join directory and filename
char* join_paths(const char* dir, const char* filename) {
    size_t dir_len = strlen(dir);
    size_t filename_len = strlen(filename);
    int need_separator = dir_len > 0 && dir[dir_len - 1] != '/';
    
    char* full_path = malloc(dir_len + filename_len + (need_separator ? 2 : 1));
    if (!full_path) {
        return NULL;
    }
    
    strcpy(full_path, dir);
    if (need_separator) {
        strcat(full_path, "/");
    }
    strcat(full_path, filename);
    
    return full_path;
}

// Check if a file exists
int file_exists(const char* file_path) {
    struct stat buffer;
    return stat(file_path, &buffer) == 0;
}

// Get system path for config file
const char* getSystemConfigFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_STATUS_FILE);
    return path;
}

// Get card file path
const char* get_card_file_path(void) {
    static char path[256];
    sprintf(path, "%s", PROD_CARD_FILE);
    return path;
}

// Get card file path with mode detection
const char* getCardFilePath(void) {
    static char path[256];
    sprintf(path, "%s", isTestingMode() ? TEST_CARD_FILE : PROD_CARD_FILE);
    return path;
}

// Get accounting file path
const char* getAccountingFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_ACCOUNTING_FILE);
    return path;
}

// Get customer file path
const char* get_customer_file_path(void) {
    static char path[256];
    sprintf(path, "%s", PROD_CUSTOMER_FILE);
    return path;
}

// Get withdrawals log file path
const char* getWithdrawalsLogFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_WITHDRAWALS_LOG_FILE);
    return path;
}

// Get transactions log file path
const char* getTransactionsLogFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_TRANSACTIONS_LOG_FILE);
    return path;
}

// Get virtual wallet file path
const char* getVirtualWalletFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_DATA_DIR);
    strcat(path, "/virtual_wallet.txt");
    return path;
}

// Get ATM config file path
const char* getATMConfigFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_DATA_DIR);
    strcat(path, "/atm_config.txt");
    return path;
}

// Get languages file path
const char* getLanguagesFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_DATA_DIR);
    strcat(path, "/languages.txt");
    return path;
}

// Get error log file path
const char* getErrorLogFilePath(void) {
    static char path[256];
    sprintf(path, "%s", isTestingMode() ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE);
    return path;
}

// Get audit log file path
const char* getAuditLogFilePath(void) {
    static char path[256];
    sprintf(path, "%s", isTestingMode() ? TEST_AUDIT_LOG_FILE : PROD_AUDIT_LOG_FILE);
    return path;
}

// Get admin credentials file path
const char* getAdminCredentialsFilePath(void) {
    static char path[256];
    sprintf(path, "%s", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    strcat(path, "/admin_credentials.txt");
    return path;
}

// Get status file path
const char* getStatusFilePath(void) {
    static char path[256];
    sprintf(path, "%s", PROD_STATUS_FILE);
    return path;
}

// Get customer file path (camelCase version)
const char* getCustomerFilePath(void) {
    return get_customer_file_path();
}

// Get UPI data file path
const char* getUPIDataFilePath(void) {
    static char path[256];
    sprintf(path, "%s", isTestingMode() ? TEST_DATA_DIR : PROD_DATA_DIR);
    strcat(path, "/upi_data.txt");
    return path;
}

// Create directory if it doesn't exist
int ensure_directory_exists(const char* dir_path) {
    struct stat st = {0};
    
    if (stat(dir_path, &st) == -1) {
        // Directory doesn't exist, try to create it
        int result = mkdir(dir_path, 0777);
        if (result != 0) {
            char error_msg[256];
            sprintf(error_msg, "Failed to create directory: %s", dir_path);
            writeErrorLog(error_msg);
            return 0;
        }
    }
    
    return 1;
}

// Initialize data files
int initialize_data_files(void) {
    char* data_files[] = {
        "data/card.txt",
        "data/customer.txt",
        "data/accounting.txt",
        "data/admin_credentials.txt",
        "data/system_config.txt",
        "data/atm_config.txt",
        "data/languages.txt",
        "data/virtual_wallet.txt",
        "data/atm_data.txt"
    };
    
    int num_files = sizeof(data_files) / sizeof(data_files[0]);
    int success = 1;
    
    for (int i = 0; i < num_files; i++) {
        FILE* file = fopen(data_files[i], "r");
        if (!file) {
            // File doesn't exist, create it
            file = fopen(data_files[i], "w");
            if (!file) {
                char error_msg[256];
                sprintf(error_msg, "Failed to create file: %s", data_files[i]);
                writeErrorLog(error_msg);
                success = 0;
                continue;
            }
            
            // Add header lines to the file
            if (strcmp(data_files[i], "data/card.txt") == 0) {
                fprintf(file, "Card DB Format Version: 1.0\n");
                fprintf(file, "Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash\n");
            } else if (strcmp(data_files[i], "data/customer.txt") == 0) {
                fprintf(file, "Customer DB Format Version: 1.0\n");
                fprintf(file, "Customer ID | Name | Card Number | Address | Phone | Email\n");
            } else if (strcmp(data_files[i], "data/accounting.txt") == 0) {
                fprintf(file, "Accounting DB Format Version: 1.0\n");
                fprintf(file, "Account ID | Card Number | Balance | Currency | Status\n");
            } else if (strcmp(data_files[i], "data/admin_credentials.txt") == 0) {
                fprintf(file, "Admin DB Format Version: 1.0\n");
                fprintf(file, "Admin ID | Username | Password Hash | Role | Last Login\n");
                fprintf(file, "1 | admin | 8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918 | SuperAdmin | 2023-01-01\n");
            }
            
            char log_msg[256];
            sprintf(log_msg, "Created data file: %s", data_files[i]);
            writeInfoLog(log_msg);
        }
        
        if (file) {
            fclose(file);
        }
    }
    
    return success;
}