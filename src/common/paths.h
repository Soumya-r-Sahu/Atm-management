#ifndef PATHS_H
#define PATHS_H

// Test mode flag
int isTestingMode();
void setTestingMode(int isTest);

// Data directory paths
#define PROD_DATA_DIR "data"
#define TEST_DATA_DIR "testing"

// Log directory paths
#define PROD_LOG_DIR "logs"
#define TEST_LOG_DIR "testing"

// File paths for production mode
#define PROD_CARD_FILE "data/card.txt"
#define PROD_CUSTOMER_FILE "data/customer.txt"
#define PROD_ACCOUNTING_FILE "data/accounting.txt"
#define PROD_VIRTUAL_WALLET_FILE "data/virtual_wallet.txt"
#define PROD_ADMIN_CREDENTIALS_FILE "data/admin_credentials.txt"
#define PROD_ATM_CONFIG_FILE "data/atm_config.txt"
#define PROD_SYSTEM_CONFIG_FILE "data/system_config.txt"
#define PROD_SECURITY_LOGS_FILE "data/security_logs.txt"
#define PROD_PIN_ATTEMPTS_FILE "data/pin_attempts.txt"
#define PROD_CARD_LOCKOUT_FILE "data/card_lockouts.txt"

// Log file paths for production mode
#define PROD_AUDIT_LOG_FILE "logs/audit.log"
#define PROD_ERROR_LOG_FILE "logs/error.log"
#define PROD_TRANSACTIONS_LOG_FILE "logs/transactions.log"
#define PROD_WITHDRAWALS_LOG_FILE "logs/withdrawals.log"

// File paths for test mode
#define TEST_CARD_FILE "testing/test_card.txt"
#define TEST_CUSTOMER_FILE "testing/test_customer.txt"
#define TEST_ACCOUNTING_FILE "testing/test_account.txt"
#define TEST_VIRTUAL_WALLET_FILE "testing/test_virtual_wallet.txt"
#define TEST_ADMIN_CREDENTIALS_FILE "testing/test_admin_credentials.txt"
#define TEST_SYSTEM_CONFIG_FILE "testing/test_system_config.txt"
#define TEST_SECURITY_LOGS_FILE "testing/test_security_logs.txt"
#define TEST_PIN_ATTEMPTS_FILE "testing/test_pin_attempts.txt"
#define TEST_CARD_LOCKOUT_FILE "testing/test_card_lockouts.txt"

// Log file paths for test mode
#define TEST_AUDIT_LOG_FILE "testing/test_audit_log.txt"
#define TEST_ERROR_LOG_FILE "testing/test_error_log.txt"
#define TEST_TRANSACTIONS_LOG_FILE "testing/test_transaction.txt"
#define TEST_WITHDRAWALS_LOG_FILE "testing/test_withdrawals.log"

// Configuration keys
#define CONFIG_MAX_WRONG_PIN_ATTEMPTS "max_wrong_pin_attempts"
#define CONFIG_PIN_LOCKOUT_MINUTES "pin_lockout_minutes"
#define CONFIG_SESSION_TIMEOUT_SECONDS "session_timeout_seconds"

// Get file paths with mode detection
const char* getCardFilePath();
const char* getCustomerFilePath();
const char* getAccountingFilePath();
const char* getVirtualWalletFilePath();
const char* getAdminCredentialsFilePath();
const char* getSystemConfigFilePath();
const char* getSecurityLogsFilePath();
const char* getPinAttemptsFilePath();
const char* getCardLockoutFilePath();

// Get log paths with mode detection
const char* getAuditLogFilePath();
const char* getErrorLogFilePath();
const char* getTransactionsLogFilePath();
const char* getWithdrawalsLogFilePath();

// Create a temporary file path
char* createTempFilePath(const char* baseFilePath);

// Join path components safely
char* joinPaths(const char* dir, const char* filename);

// Ensure directory exists
int ensureDirectoryExists(const char* dirPath);

// Initialize directories and files
int initializeDataFiles();

#endif // PATHS_H