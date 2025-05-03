/**
 * @file db_config.h
 * @brief Database configuration and connection pool for the ATM Management System
 * @version 1.0
 * @date May 3, 2025
 */

#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <stdbool.h>
#include <mysql/mysql.h>

// Database connection settings
#define DB_HOST "localhost"
#define DB_USER "atm_app"
#define DB_PASS "secure_password"
#define DB_NAME "atm_management"

// Table names
#define TABLE_CUSTOMERS "customers"
#define TABLE_ACCOUNTS "accounts"
#define TABLE_CARDS "cards"
#define TABLE_TRANSACTIONS "transactions"
#define TABLE_DAILY_WITHDRAWALS "daily_withdrawals"
#define TABLE_BILL_PAYMENTS "bill_payments"
#define TABLE_TRANSFERS "transfers"
#define TABLE_AUDIT_LOGS "audit_logs"
#define TABLE_SYSTEM_LOGS "system_logs"
#define TABLE_ADMIN_USERS "admin_users"
#define TABLE_ATM_MACHINES "atm_machines"
#define TABLE_ATM_CASH_OPERATIONS "atm_cash_operations"
#define TABLE_SESSIONS "sessions"
#define TABLE_DB_VERSION "db_version"

// Common column names
#define COL_CUSTOMER_ID "customer_id"
#define COL_ACCOUNT_NUMBER "account_number"
#define COL_CARD_ID "card_id"
#define COL_CARD_NUMBER "card_number"
#define COL_TRANSACTION_ID "transaction_id"
#define COL_STATUS "status"
#define COL_TIMESTAMP "timestamp"
#define COL_AMOUNT "amount"
#define COL_BALANCE "balance"

// Connection pool settings
#define MAX_DB_CONNECTIONS 10
#define CONNECTION_TIMEOUT 5 // seconds

// Transaction settings
#define MAX_TRANSACTION_RETRIES 3
#define TRANSACTION_RETRY_DELAY 1000 // milliseconds

// Log settings
#define MAX_LOG_QUERY_LENGTH 2048
#define MAX_LOG_ENTRIES_PER_PAGE 100

// Structure to represent a connection in the pool
typedef struct {
    MYSQL* connection;
    bool in_use;
    time_t last_used;
} DatabaseConnection;

/**
 * Initialize the database connection pool
 * @return true if successful, false otherwise
 */
bool db_init(void);

/**
 * Clean up and release database connection pool resources
 */
void db_cleanup(void);

/**
 * Get a connection from the pool
 * @return Connection handle or NULL if none available
 */
MYSQL* db_get_connection(void);

/**
 * Release a connection back to the pool
 * @param conn Connection handle to release
 */
void db_release_connection(MYSQL* conn);

/**
 * Execute a simple SQL query with no result set
 * @param query SQL query to execute
 * @return true if successful, false otherwise
 */
bool db_execute_query(const char* query);

/**
 * Execute a SQL query that returns a result set
 * @param query SQL query to execute
 * @param callback Function to call for each row in the result
 * @param user_data User data to pass to the callback
 * @return true if successful, false otherwise
 */
bool db_execute_select(const char* query, 
                       void (*callback)(MYSQL_ROW row, void* user_data), 
                       void* user_data);

/**
 * Check if database connection is active
 * @return true if connected, false otherwise
 */
bool db_is_connected(void);

/**
 * Get the number of active connections in the pool
 * @return Number of active connections
 */
int db_get_active_connections(void);

/**
 * Get the last error message from the database
 * @return Error message string
 */
const char* db_get_error(void);

/**
 * Prepare and execute a parameterized query using prepared statements
 * @param query SQL query template with ? placeholders
 * @param bind_count Number of parameters to bind
 * @param ... Variable arguments for parameters
 * @return true if successful, false otherwise
 */
bool db_prepared_query(const char* query, int bind_count, ...);

/**
 * Begin a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_begin_transaction(MYSQL* conn);

/**
 * Commit a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_commit_transaction(MYSQL* conn);

/**
 * Rollback a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_rollback_transaction(MYSQL* conn);

/**
 * Escape a string to prevent SQL injection
 * @param input Input string to escape
 * @param output Output buffer for escaped string
 * @param size Size of output buffer
 * @return true if successful, false otherwise
 */
bool db_escape_string(const char* input, char* output, size_t size);

#endif /* DB_CONFIG_H */