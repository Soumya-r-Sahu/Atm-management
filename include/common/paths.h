#ifndef PATHS_H
#define PATHS_H

// Test mode flag
int isTestingMode();
void setTestingMode(int isTest);

// Data directories
#define PROD_DATA_DIR "data"
#define TEST_DATA_DIR "data/test"

// Card data files
#define PROD_CARD_FILE "data/card.txt"
#define TEST_CARD_FILE "data/test/card.txt"

// Customer data files
#define PROD_CUSTOMER_FILE "data/customer.txt"
#define TEST_CUSTOMER_FILE "data/test/customer.txt"

// Accounting data files
#define PROD_ACCOUNTING_FILE "data/accounting.txt"
#define TEST_ACCOUNTING_FILE "data/test/accounting.txt"

// Admin credentials files
#define PROD_ADMIN_CREDENTIALS_FILE "data/admin_credentials.txt"
#define TEST_ADMIN_CREDENTIALS_FILE "data/test/admin_credentials.txt"

// Log files
#define PROD_ERROR_LOG_FILE "logs/error.log"
#define TEST_ERROR_LOG_FILE "logs/test/error.log"
#define PROD_AUDIT_LOG_FILE "logs/audit.log"
#define TEST_AUDIT_LOG_FILE "logs/test/audit.log"
#define PROD_TRANSACTIONS_LOG_FILE "logs/transactions.log"
#define TEST_TRANSACTIONS_LOG_FILE "logs/test/transactions.log"
#define PROD_WITHDRAWALS_LOG_FILE "logs/withdrawals.log"
#define TEST_WITHDRAWALS_LOG_FILE "logs/test/withdrawals.log"

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