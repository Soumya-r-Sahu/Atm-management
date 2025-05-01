#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

// Path constants for data files
#define CARD_DATA_FILE "data/card.txt"
#define CUSTOMER_DATA_FILE "data/customer.txt"
#define ACCOUNTING_DATA_FILE "data/accounting.txt"
#define ADMIN_CREDENTIALS_FILE "data/admin_credentials.txt"
#define ATM_DATA_FILE "data/atm_data.txt"
#define ATM_TRANSACTIONS_FILE "data/atm_transactions.txt"
#define VIRTUAL_WALLET_FILE "data/virtual_wallet.txt"

// Path constants for logs
#define PROD_ERROR_LOG_FILE "logs/error.log"
#undef TEST_ERROR_LOG_FILE
#define TEST_ERROR_LOG_FILE "testing/test_error.log"
#define PROD_AUDIT_LOG_FILE "logs/audit.log"
#undef TEST_AUDIT_LOG_FILE
#define TEST_AUDIT_LOG_FILE "testing/test_audit.log"
#define PROD_TRANSACTIONS_LOG_FILE "logs/transactions.log"
#undef TEST_TRANSACTIONS_LOG_FILE
#define TEST_TRANSACTIONS_LOG_FILE "testing/test_transactions.log"

// Path constants for configuration
#define SYSTEM_CONFIG_FILE "data/system_config.txt"
#define ATM_CONFIG_FILE "data/atm_config.txt"
#define LANGUAGE_FILE "data/languages.txt"

// Session timeout constants
#undef CONFIG_SESSION_TIMEOUT_SECONDS
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"
#define DEFAULT_SESSION_TIMEOUT 180

/**
 * Set the testing mode
 * @param isTestMode 1 for test mode, 0 for production mode
 */
void setTestingMode(int isTestMode);

/**
 * Check if testing mode is enabled
 * @return 1 if in test mode, 0 if in production mode
 */
int isTestingMode();

/**
 * Ensure a directory exists
 * @param path The directory path to check/create
 * @return 1 if directory exists or was created, 0 if failed
 */
int ensureDirectoryExists(const char* path);

/**
 * Get the path to the card file
 * @return The path to the card file based on test mode
 */
const char* getCardFilePath();

/**
 * Get the path to the customer file
 * @return The path to the customer file based on test mode
 */
const char* getCustomerFilePath();

/**
 * Get the path to the accounting file
 * @return The path to the accounting file based on test mode
 */
const char* getAccountingFilePath();

/**
 * Get the path to the admin credentials file
 * @return The path to the admin credentials file based on test mode
 */
const char* getAdminCredentialsFilePath();

/**
 * Get the path to the system config file
 * @return The path to the system config file based on test mode
 */
const char* getSystemConfigFilePath();

/**
 * Get the path to the transaction log file
 * @return The path to the transaction log file based on test mode
 */
const char* getTransactionLogFilePath();

/**
 * Get the path to the error log file
 * @return The path to the error log file based on test mode
 */
const char* getErrorLogFilePath();

/**
 * Get the path to the audit log file
 * @return The path to the audit log file based on test mode
 */
const char* getAuditLogFilePath();

/**
 * Get the path to the withdrawal log file
 * @return The path to the withdrawal log file based on test mode
 */
const char* getWithdrawalLogFilePath();

// File path getter functions
const char* get_card_file_path(void);
const char* get_customer_file_path(void);
const char* get_transactions_file_path(void);
const char* get_system_config_file_path(void);
const char* get_error_log_file_path(void);
const char* get_audit_log_file_path(void);

// Additional path getter functions
const char* getAccountingFilePath(void);
const char* getTransactionsLogFilePath(void);
const char* getWithdrawalsLogFilePath(void);

// Path helper functions
char* create_temp_file_path(const char* base_file_path);
char* join_paths(const char* dir, const char* filename);
int ensure_directory_exists(const char* dir_path);
int file_exists(const char* file_path);
int ensure_file_exists(const char* file_path);
int initialize_data_files(void);

#endif // PATH_MANAGER_H