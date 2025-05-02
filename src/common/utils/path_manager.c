#include "common/utils/path_manager.h"
#include "common/utils/file_utils.h"
#include "common/utils/string_utils.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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

// Create a directory if it doesn't exist
int ensure_directory_exists(const char* dir_path) {
    struct stat st;
    if (stat(dir_path, &st) == 0) {
        // Directory already exists
        return 1;
    }
    
    // Create the directory
#ifdef _WIN32
    int result = mkdir(dir_path);
#else
    int result = mkdir(dir_path, 0755); // rwxr-xr-x
#endif
    
    if (result == 0) {
        return 1; // Directory created successfully
    }
    
    // Check for EEXIST (directory already exists) which is fine
    if (errno == EEXIST) {
        return 1;
    }
    
    // Failed to create directory for some other reason
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Failed to create directory %s: %s", dir_path, strerror(errno));
    write_error_log(error_msg);
    return 0;
}

// Ensure a file exists (create if doesn't exist)
int ensure_file_exists(const char* file_path) {
    if (file_exists(file_path)) {
        return 1; // File already exists
    }
    
    // Create the file
    FILE* file = fopen(file_path, "w");
    if (!file) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Failed to create file %s: %s", file_path, strerror(errno));
        write_error_log(error_msg);
        return 0;
    }
    
    fclose(file);
    return 1;
}

// File path getters
const char* get_card_file_path(void) {
    return g_is_test_mode ? "test/data/card.txt" : CARD_DATA_FILE;
}

const char* get_customer_file_path(void) {
    return g_is_test_mode ? "test/data/customer.txt" : CUSTOMER_DATA_FILE;
}

const char* get_transactions_file_path(void) {
    return g_is_test_mode ? "test/data/atm_transactions.txt" : ATM_TRANSACTIONS_FILE;
}

const char* get_system_config_file_path(void) {
    return g_is_test_mode ? "test/data/system_config.txt" : SYSTEM_CONFIG_FILE;
}

const char* get_error_log_file_path(void) {
    return g_is_test_mode ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE;
}

const char* get_audit_log_file_path(void) {
    return g_is_test_mode ? TEST_AUDIT_LOG_FILE : PROD_AUDIT_LOG_FILE;
}

// Additional path getters
const char* getAccountingFilePath(void) {
    return g_is_test_mode ? "test/data/accounting.txt" : ACCOUNTING_DATA_FILE;
}

const char* getTransactionsLogFilePath(void) {
    return g_is_test_mode ? TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE;
}

const char* getWithdrawalsLogFilePath(void) {
    return g_is_test_mode ? "test/logs/withdrawals.log" : "logs/withdrawals.log";
}

// Additional path getters needed by admin system
const char* getErrorLogFilePath(void) {
    return get_error_log_file_path();
}

const char* getAuditLogFilePath(void) {
    return get_audit_log_file_path();
}

const char* getStatusFilePath(void) {
    return g_is_test_mode ? "test/data/service_status.txt" : "data/service_status.txt";
}

// Alternative function names with camelCase for compatibility
const char* getCardFilePath(void) {
    return get_card_file_path();
}

const char* getCustomerFilePath(void) {
    return get_customer_file_path();
}

const char* getSystemConfigFilePath(void) {
    return get_system_config_file_path();
}

const char* getAdminCredentialsFilePath(void) {
    return g_is_test_mode ? "test/data/admin_credentials.txt" : ADMIN_CREDENTIALS_FILE;
}

int ensureDirectoryExists(const char* dir_path) {
    return ensure_directory_exists(dir_path);
}

// Initialize required data files
int initialize_data_files(void) {
    // Ensure data directories exist
    if (!ensure_directory_exists("data")) {
        return 0;
    }
    
    if (!ensure_directory_exists("logs")) {
        return 0;
    }
    
    if (g_is_test_mode) {
        if (!ensure_directory_exists("test")) return 0;
        if (!ensure_directory_exists("test/data")) return 0;
        if (!ensure_directory_exists("test/logs")) return 0;
    }
    
    // Create basic structure for essential files if they don't exist
    const char* card_file = get_card_file_path();
    if (!file_exists(card_file)) {
        FILE* file = fopen(card_file, "w");
        if (!file) return 0;
        fprintf(file, "Card Database\n");
        fprintf(file, "Card ID | Account ID | Card Number | Card Type | Expiry Date | Status | PIN Hash\n");
        fclose(file);
    }
    
    const char* customer_file = get_customer_file_path();
    if (!file_exists(customer_file)) {
        FILE* file = fopen(customer_file, "w");
        if (!file) return 0;
        fprintf(file, "Customer Database\n");
        fprintf(file, "Customer ID | Name | Card Number | Address | Phone | Email\n");
        fclose(file);
    }
    
    const char* accounting_file = getAccountingFilePath();
    if (!file_exists(accounting_file)) {
        FILE* file = fopen(accounting_file, "w");
        if (!file) return 0;
        fprintf(file, "Accounting Database\n");
        fprintf(file, "Account ID | Card Number | Balance | Currency | Status\n");
        fclose(file);
    }
    
    return 1;
}