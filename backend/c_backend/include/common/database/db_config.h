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

// Database connection parameters
#define DB_HOST "localhost"
#define DB_USER "atm_user"
#define DB_PASS "securepassword"
#define DB_NAME "atm_management"
#define DB_PORT 3306
#define DB_CONNECT_TIMEOUT 5 // In seconds

// Maximum number of connections in the pool
#define MAX_DB_CONNECTIONS 10

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
void* db_get_connection(void);

/**
 * Release a connection back to the pool
 * @param conn Connection handle to release
 */
void db_release_connection(void* conn);

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