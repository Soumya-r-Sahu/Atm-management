/**
 * @file db_config.h
 * @brief Database configuration and connection pool for the ATM Management System
 * @version 2.0
 * @date May 9, 2025
 */

#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <stdbool.h>
#include <time.h>

#ifdef NO_MYSQL
    #include "mysql_stub.h"
    #define USING_MYSQL_STUB 1
    #pragma message("Building with MySQL stub implementation")
#else
    // Try different paths for MySQL headers based on OS
    #if defined(_WIN32) || defined(_WIN64)
        // Windows paths
        #if __has_include(<mysql.h>)
            #include <mysql.h>
            #undef MYSQL_HEADER_NOT_FOUND
        #elif __has_include(<mysql/mysql.h>)
            #include <mysql/mysql.h>
            #undef MYSQL_HEADER_NOT_FOUND
        #elif __has_include("C:/Program Files/MySQL/MySQL Server 8.0/include/mysql.h")
            #include "C:/Program Files/MySQL/MySQL Server 8.0/include/mysql.h"
            #undef MYSQL_HEADER_NOT_FOUND
        #endif
    #else
        // Linux/Mac paths
        #if __has_include(<mysql/mysql.h>)
            #include <mysql/mysql.h>
            #undef MYSQL_HEADER_NOT_FOUND
        #elif __has_include(<mysql.h>)
            #include <mysql.h>
            #undef MYSQL_HEADER_NOT_FOUND
        #elif __has_include("/usr/include/mysql/mysql.h")
    #include "/usr/include/mysql/mysql.h"
    #undef MYSQL_HEADER_NOT_FOUND
  #elif __has_include("/usr/local/include/mysql/mysql.h")
    #include "/usr/local/include/mysql/mysql.h"
    #undef MYSQL_HEADER_NOT_FOUND
  #endif
#endif

// Create comprehensive MySQL stub implementations if headers weren't found
#ifdef MYSQL_HEADER_NOT_FOUND
  #warning "MySQL headers not found. Using stub implementations for compilation only."
  
  // Basic MySQL typedefs
  typedef void MYSQL;
  typedef void MYSQL_RES;
  typedef char** MYSQL_ROW;
  typedef unsigned long long MYSQL_FIELD;
  typedef unsigned long MYSQL_FIELD_OFFSET;
  typedef unsigned int my_bool;
  typedef unsigned long long my_ulonglong;
  
  // Connection handling
  #define mysql_init(x) ((MYSQL*)NULL)
  #define mysql_real_connect(a,b,c,d,e,f,g,h) ((MYSQL*)NULL)
  #define mysql_close(x)
  #define mysql_ping(x) (-1)
  #define mysql_select_db(a,b) (-1)
  #define mysql_change_user(a,b,c,d) (-1)
  
  // Error handling
  #define mysql_error(x) "MySQL headers not found - stub implementation"
  #define mysql_errno(x) (-1)
  #define mysql_sqlstate(x) "HY000"
  
  // Query execution
  #define mysql_query(a,b) (-1)
  #define mysql_real_query(a,b,c) (-1)
  
  // Result set handling
  #define mysql_store_result(x) ((MYSQL_RES*)NULL)
  #define mysql_use_result(x) ((MYSQL_RES*)NULL)
  #define mysql_free_result(x)
  #define mysql_fetch_row(x) ((MYSQL_ROW)NULL)
  #define mysql_fetch_lengths(x) ((unsigned long*)NULL)
  #define mysql_fetch_field(x) ((MYSQL_FIELD*)NULL)
  #define mysql_data_seek(a,b)
  #define mysql_row_seek(a,b) ((MYSQL_ROW_OFFSET)NULL)
  #define mysql_field_seek(a,b) ((MYSQL_FIELD_OFFSET)NULL)
  
  // Information functions
  #define mysql_num_rows(x) ((my_ulonglong)0)
  #define mysql_num_fields(x) ((unsigned int)0)
  #define mysql_affected_rows(x) ((my_ulonglong)0)
  #define mysql_insert_id(x) ((my_ulonglong)0)
  #define mysql_field_count(x) ((unsigned int)0)
  
  // String escaping
  #define mysql_real_escape_string(a,b,c,d) ((unsigned long)0)
  #define mysql_hex_string(a,b,c) ((unsigned long)0)
  
  // Transaction handling
  #define mysql_autocommit(a,b) (-1)
  #define mysql_commit(x) (-1)
  #define mysql_rollback(x) (-1)
  
  // Prepared statements (basic stubs)
  typedef void MYSQL_STMT;
  typedef void MYSQL_BIND;
  #define mysql_stmt_init(x) ((MYSQL_STMT*)NULL)
  #define mysql_stmt_prepare(a,b,c) (-1)
  #define mysql_stmt_execute(x) (-1)
  #define mysql_stmt_close(x) (0)
  #define mysql_stmt_bind_param(a,b) (-1)
  #define mysql_stmt_bind_result(a,b) (-1)
  #define mysql_stmt_fetch(x) (-1)
  #define mysql_stmt_free_result(x) (0)
  #define mysql_stmt_error(x) "MySQL headers not found - stub implementation"
  #define mysql_stmt_errno(x) (-1)
  
  // Character set functions
  #define mysql_character_set_name(x) "utf8"
  #define mysql_set_character_set(a,b) (-1)
  
  // Server information
  #define mysql_get_server_info(x) "MySQL Stub 0.0.0"
  #define mysql_get_client_info() "MySQL Stub Client 0.0.0"
  #define mysql_get_client_version() 0
#endif

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
 * Get a connection from the pool with optimized search algorithm
 * This improved version has O(1) best case and O(n) worst case
 * @return Connection handle or NULL if none available
 */
MYSQL* db_get_connection(void);

/**
 * Release a connection back to the pool with O(1) complexity
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