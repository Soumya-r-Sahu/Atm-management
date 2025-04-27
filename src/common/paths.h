#ifndef PATHS_H
#define PATHS_H

#include <stdbool.h>

// Directory paths
#define PROD_DATA_DIR "./../../data"
#define TEST_DATA_DIR "./../../testing"

// File paths for production
#define PROD_CREDENTIALS_FILE PROD_DATA_DIR "/credentials.txt"
#define PROD_ACCOUNTING_FILE PROD_DATA_DIR "/accounting.txt"
#define PROD_ADMIN_CRED_FILE PROD_DATA_DIR "/admin_credentials.txt"
#define PROD_AUDIT_LOG_FILE PROD_DATA_DIR "/audit.log"
#define PROD_STATUS_FILE PROD_DATA_DIR "/status.txt"
#define PROD_LANGUAGES_FILE PROD_DATA_DIR "/languages.txt"

// File paths for testing
#define TEST_CREDENTIALS_FILE TEST_DATA_DIR "/test_card.txt"
#define TEST_ACCOUNTING_FILE TEST_DATA_DIR "/test_account.txt"
#define TEST_ADMIN_CRED_FILE TEST_DATA_DIR "/test_admin_credentials.txt"
#define TEST_AUDIT_LOG_FILE TEST_DATA_DIR "/test_audit_log.txt"
#define TEST_STATUS_FILE TEST_DATA_DIR "/test_system_config.txt"
#define TEST_LANGUAGES_FILE TEST_DATA_DIR "/test_languages.txt"

// Transaction log files
#define PROD_TRANSACTIONS_LOG_FILE PROD_DATA_DIR "/../logs/transactions.log"
#define TEST_TRANSACTIONS_LOG_FILE TEST_DATA_DIR "/test_transaction.txt"

// Error log files
#define PROD_ERROR_LOG_FILE PROD_DATA_DIR "/../logs/error.log"
#define TEST_ERROR_LOG_FILE TEST_DATA_DIR "/test_security_logs.txt"

// Dynamic file paths based on environment
#define CREDENTIALS_FILE (isTestingMode() ? TEST_CREDENTIALS_FILE : PROD_CREDENTIALS_FILE)
#define ACCOUNTING_FILE (isTestingMode() ? TEST_ACCOUNTING_FILE : PROD_ACCOUNTING_FILE)
#define ADMIN_CRED_FILE (isTestingMode() ? TEST_ADMIN_CRED_FILE : PROD_ADMIN_CRED_FILE)
#define AUDIT_LOG_FILE (isTestingMode() ? TEST_AUDIT_LOG_FILE : PROD_AUDIT_LOG_FILE)
#define STATUS_FILE (isTestingMode() ? TEST_STATUS_FILE : PROD_STATUS_FILE)
#define LANGUAGES_FILE (isTestingMode() ? TEST_LANGUAGES_FILE : PROD_LANGUAGES_FILE)
#define TRANSACTIONS_LOG_FILE (isTestingMode() ? TEST_TRANSACTIONS_LOG_FILE : PROD_TRANSACTIONS_LOG_FILE)
#define ERROR_LOG_FILE (isTestingMode() ? TEST_ERROR_LOG_FILE : PROD_ERROR_LOG_FILE)

/**
 * Checks if the application is currently in testing mode
 * 
 * @return true if running in test mode, false otherwise
 */
bool isTestingMode();

/**
 * Sets the application to run in testing or production mode
 * 
 * @param testing true to use test data, false to use production data
 */
void setTestingMode(bool testing);

/**
 * Creates any missing data files and directories
 * 
 * @return true if all required files and directories are ready, false otherwise
 */
bool initializeDataFiles();

/**
 * Gets the path to the card file based on test/production mode
 * 
 * @return path to the card file
 */
const char* getCardFilePath();

/**
 * Gets the path to the customer file based on test/production mode
 * 
 * @return path to the customer file
 */
const char* getCustomerFilePath();

#endif // PATHS_H