# CBS Project - Function Reference

This document provides a comprehensive reference for all major functions in the Core Banking System project, organized by component.

## Table of Contents

1. [Database Access Layer](#database-access-layer)
2. [Logging System](#logging-system)
3. [Security System](#security-system)
4. [Transaction Processing](#transaction-processing)
5. [Admin Operations](#admin-operations)
6. [Testing Framework](#testing-framework)

## Database Access Layer

### DAO Interface

```c
// DAO creation functions
DatabaseAccessObject* createMySQLDAO(void);
DatabaseAccessObject* createFileBasedDAO(void);
DatabaseAccessObject* createMySQLPooledDAO(void);
DatabaseAccessObject* getDAO(void);
void releaseDAO(void);

// DAO interface functions
bool doesCardExist(int cardNumber);
bool isCardActive(int cardNumber);
bool validateCard(int cardNumber, int pin);
bool validateCardWithHash(int cardNumber, const char* pinHash);
bool validateCardCVV(int cardNumber, int cvv);
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);
bool updateCardPIN(int cardNumber, const char* newPINHash);
bool getCardHolderName(int cardNumber, char* name, size_t nameSize);
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);
float fetchBalance(int cardNumber);
bool updateBalance(int cardNumber, float newBalance);
float getDailyWithdrawals(int cardNumber);
void logWithdrawal(int cardNumber, float amount);
bool logTransaction(int cardNumber, const char* transactionType, float amount, bool success);
bool getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count);
void* getConnection(void);
void releaseConnection(void* conn);
```

### Connection Pool

```c
// Connection pool creation and destruction
ConnectionPool* connection_pool_create(const ConnectionPoolConfig* config);
void connection_pool_destroy(ConnectionPool* pool);

// Connection management
DatabaseConnection* connection_pool_get(ConnectionPool* pool);
bool connection_pool_return(DatabaseConnection* conn);
bool connection_pool_release(DatabaseConnection* conn);

// Pool statistics and monitoring
void connection_pool_stats(ConnectionPool* pool, 
                          int* active_connections, 
                          int* idle_connections, 
                          int* total_connections,
                          int* connection_requests, 
                          double* avg_wait_time_ms);
```

## Logging System

### CBS Logger

```c
// Initialization and shutdown
bool cbs_initializeLogger(const char* logDirectory);
void cbs_shutdownLogger(void);

// Log level management
void cbs_setLogLevel(CBSLogLevel level);
CBSLogLevel cbs_getLogLevel(void);
const char* cbs_logLevelToString(CBSLogLevel level);

// Basic logging
void cbs_writeLog(LogCategory category, CBSLogLevel level, const char* format, ...);
void cbs_writeLogWithLocation(LogCategory category, CBSLogLevel level,
                            const char* file, int line, const char* function,
                            const char* format, ...);

// Specialized logging
void cbs_writeTransactionLog(const char* transactionId, const char* userId,
                           const char* cardNumber, const char* accountId,
                           const char* transactionType, double amount,
                           double previousBalance, double newBalance,
                           const char* status, const char* details);

void cbs_writeSecurityLog(const char* userId, const char* eventType,
                        const char* severity, const char* status,
                        const char* targetResource, const char* details,
                        const char* sourceIP);

// PII data protection
void cbs_maskCardNumber(const char* cardNumber, char* masked, size_t maskedSize);
void cbs_maskPII(const char* data, char* masked, size_t maskedSize);

// Convenience macros
#define CBS_LOG_DEBUG(category, format, ...)
#define CBS_LOG_INFO(category, format, ...)
#define CBS_LOG_WARNING(category, format, ...)
#define CBS_LOG_ERROR(category, format, ...)
#define CBS_LOG_CRITICAL(category, format, ...)
```

### DAO Audit Logger

```c
// Initialization and shutdown
bool init_dao_audit_logger(const char* logDirectory, int retentionDays);
void close_dao_audit_logger(void);

// Transaction auditing
void record_transaction_audit(const char* userId, const char* cardNumber,
                            const char* transactionType, float amount,
                            float oldBalance, float newBalance,
                            bool success);

// Card operation auditing
void record_card_operation_audit(const char* userId, const char* cardNumber,
                               const char* operationType,
                               const char* oldStatus, const char* newStatus,
                               bool success);

// Database operation auditing
void record_db_operation_audit(const char* operationType, const char* tableName,
                             const char* recordId, const char* beforeState,
                             const char* afterState, bool success,
                             const char* errorMessage);

// Authentication auditing
void record_auth_audit(const char* userId, const char* authType,
                      bool success, const char* clientIp,
                      const char* deviceInfo);

// Enhanced transaction logging
void enhanced_transaction_log(const char* cardNumber, const char* username,
                            const char* transactionType, float amount,
                            float oldBalance, float newBalance,
                            bool success);
```

## Security System

### Secure Password System

```c
// Initialization and cleanup
bool secure_password_init(void);
void secure_password_cleanup(void);

// Password hashing and verification
char* secure_password_hash(const char* password, HashAlgorithm algorithm, int work_factor);
bool secure_password_verify(const char* password, const char* hash);
bool secure_password_needs_rehash(const char* hash);

// Token generation
char* secure_generate_token(size_t length);

// Basic hash utilities
char* sha256_hash(const char* input);
bool secure_hash_compare(const char* hash1, const char* hash2);
char* generate_salt(int length);
char* create_salted_hash(const char* password, const char* salt);
bool verify_password(const char* password, const char* salted_hash);
```

### Admin Authentication

```c
// Admin authentication
bool initialize_admin_auth(void);
bool get_admin_credentials(const char* username, char* password_hash, 
                         int max_hash_len, char* roles, int max_roles_len);
bool authenticate_admin(const char* username, const char* password, AdminUser* admin);
bool admin_has_role(const AdminUser* admin, const char* role);

// Session management
AdminUser* create_admin_session(const char* username, const char* roles[], int num_roles);
bool end_admin_session(AdminUser* admin);
bool is_admin_session_valid(AdminUser* admin);
void update_admin_activity(AdminUser* admin);

// Account management
bool change_admin_password(AdminUser* admin, const char* old_password, const char* new_password);
bool update_admin_status(AdminUser* admin, const char* target_username, bool status);
bool create_admin_account(AdminUser* admin, const char* new_username, 
                         const char* new_password, const char* roles[], int num_roles);
bool admin_username_exists(const char* username);

// Security measures
bool is_account_locked(const char* username);
bool update_login_attempts(const char* username);
bool decrement_login_attempts(const char* username);
```

## Transaction Processing

### ATM Transactions

```c
// Card operations
bool validateATMCard(int cardNumber, int pin);
bool checkATMCardStatus(int cardNumber);

// Balance operations
float getAccountBalance(int cardNumber);
bool withdrawFromAccount(int cardNumber, float amount);
bool depositToAccount(int cardNumber, float amount);

// Transaction operations
bool performFundsTransfer(int sourceCard, int targetCard, float amount);
bool requestMiniStatement(int cardNumber, Statement* statement);
bool performBillPayment(int cardNumber, const char* billerCode, float amount);

// Special operations
bool changePIN(int cardNumber, int oldPIN, int newPIN);
bool requestCardBlock(int cardNumber, BlockReason reason);
```

### Transaction Manager

```c
// Transaction processing
TransactionResult processTransaction(Transaction* transaction);
void rollbackTransaction(Transaction* transaction);
bool validateTransaction(Transaction* transaction);

// Transaction limits
bool checkWithdrawalLimit(int cardNumber, float amount);
bool checkTransactionLimit(int cardNumber, float amount);
bool updateDailyLimits(int cardNumber, float amount, TransactionType type);

// Transaction records
TransactionID generateTransactionID(void);
bool storeTransactionRecord(Transaction* transaction);
bool retrieveTransactionHistory(int cardNumber, TransactionRecord* records, int* count);
```

## Admin Operations

### System Management

```c
// User management
bool createUser(AdminUser* admin, UserDetails* newUser);
bool updateUserStatus(AdminUser* admin, const char* username, UserStatus status);
bool resetUserPassword(AdminUser* admin, const char* username);
bool getUserDetails(AdminUser* admin, const char* username, UserDetails* details);
bool listUsers(AdminUser* admin, UserListFilter filter, UserDetails** users, int* count);

// System configuration
bool getSystemConfig(AdminUser* admin, ConfigSection section, ConfigValue* values);
bool updateSystemConfig(AdminUser* admin, ConfigSection section, ConfigValue* values);
bool backupSystemConfig(AdminUser* admin, const char* backupPath);
bool restoreSystemConfig(AdminUser* admin, const char* backupPath);

// Monitoring operations
SystemStatus getSystemStatus(AdminUser* admin);
ResourceUsage getResourceUsage(AdminUser* admin);
UserActivity* getUserActivity(AdminUser* admin, const char* username, 
                            time_t startTime, time_t endTime, int* count);
```

### ATM Management

```c
// ATM configuration
bool configureATM(AdminUser* admin, int atmID, ATMConfig* config);
bool getATMConfig(AdminUser* admin, int atmID, ATMConfig* config);
bool updateATMStatus(AdminUser* admin, int atmID, ATMStatus status);
bool listATMs(AdminUser* admin, ATMListFilter filter, ATMInfo** atms, int* count);

// Cash management
bool updateATMCashBalance(AdminUser* admin, int atmID, float amounts[DENOMINATIONS]);
bool getATMCashBalance(AdminUser* admin, int atmID, float amounts[DENOMINATIONS]);
bool recordCashReplenishment(AdminUser* admin, int atmID, float amounts[DENOMINATIONS]);
bool getCashReport(AdminUser* admin, int atmID, time_t startDate, 
                 time_t endDate, CashReport* report);
```

## Testing Framework

```c
// Framework initialization
void test_init(const char* testSuiteName);
void test_setup_teardown(void (*setup)(void), void (*teardown)(void));
int test_cleanup(void);

// Test case management
void test_run(const char* name, void (*test_func)(void));
void test_skip(const char* name, const char* reason);

// Assertions
void test_assert(bool condition, const char* message);
void test_assert_int_equal(int expected, int actual, const char* message);
void test_assert_str_equal(const char* expected, const char* actual, const char* message);
void test_assert_float_equal(float expected, float actual, float epsilon, const char* message);
void test_assert_null(void* ptr, const char* message);
void test_assert_not_null(void* ptr, const char* message);

// Mock DAO
DatabaseAccessObject* getTestDAO(void);
void setupMockCard(int cardNumber, int pin, float initialBalance, bool isActive);
void clearMockData(void);
```

---

## Navigation

- [Documentation Index](./README.md)
- [Project Documentation](./CBS_PROJECT_DOCUMENTATION.md)
- [Logging Guide](./CBS_LOGGING_GUIDE.md)
- [Build Guide](./CBS_BUILD_GUIDE.md)
- [Testing Plan](./CBS_TESTING_PLAN.md)
