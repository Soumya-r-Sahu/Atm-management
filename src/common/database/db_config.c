/**
 * @file db_config.c
 * @brief Database configuration and connection pool implementation for the ATM Management System
 * @version 1.0
 * @date May 3, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/utils/logger.h"

// Connection pool
static DatabaseConnection connection_pool[MAX_DB_CONNECTIONS];

// Thread safety mutex
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;

// Connection idle timeout (in seconds)
#define CONNECTION_IDLE_TIMEOUT 300 // 5 minutes

// Forward declarations of internal functions
static MYSQL* create_connection(void);
static void close_connection(MYSQL* conn);
static void cleanup_idle_connections(void);

/**
 * Initialize the database connection pool
 * @return true if successful, false otherwise
 */
bool db_init(void) {
    pthread_mutex_lock(&pool_mutex);
    
    // Initialize MySQL library
    if (mysql_library_init(0, NULL, NULL)) {
        writeErrorLog("Failed to initialize MySQL client library");
        pthread_mutex_unlock(&pool_mutex);
        return false;
    }
    
    // Initialize the connection pool
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        connection_pool[i].connection = NULL;
        connection_pool[i].in_use = false;
        connection_pool[i].last_used = 0;
    }
    
    // Create an initial connection to verify database is reachable
    MYSQL* test_conn = create_connection();
    if (!test_conn) {
        writeErrorLog("Failed to connect to database during initialization");
        pthread_mutex_unlock(&pool_mutex);
        return false;
    }
    
    // Release the test connection
    close_connection(test_conn);
    
    writeInfoLog("Database connection pool initialized successfully");
    pthread_mutex_unlock(&pool_mutex);
    return true;
}

/**
 * Clean up and release database connection pool resources
 */
void db_cleanup(void) {
    pthread_mutex_lock(&pool_mutex);
    
    // Close all open connections
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection) {
            mysql_close(connection_pool[i].connection);
            connection_pool[i].connection = NULL;
            connection_pool[i].in_use = false;
        }
    }
    
    // Clean up MySQL library
    mysql_library_end();
    
    writeInfoLog("Database connection pool cleaned up");
    pthread_mutex_unlock(&pool_mutex);
}

/**
 * Get a connection from the pool
 * @return Connection handle or NULL if none available
 */
MYSQL* db_get_connection(void) {
    MYSQL* conn = NULL;
    int free_index = -1;
    
    pthread_mutex_lock(&pool_mutex);
    
    // First, look for an existing idle connection
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection && !connection_pool[i].in_use) {
            connection_pool[i].in_use = true;
            conn = connection_pool[i].connection;
            connection_pool[i].last_used = time(NULL);
            
            // Ping the connection to make sure it's still alive
            if (mysql_ping(conn) != 0) {
                writeWarningLog("Connection %d is dead, reconnecting", i);
                mysql_close(conn);
                conn = create_connection();
                
                if (conn) {
                    connection_pool[i].connection = conn;
                } else {
                    connection_pool[i].in_use = false;
                    conn = NULL;
                    free_index = -1;
                }
            } else {
                writeDebugLog("Reusing existing connection %d", i);
            }
            
            pthread_mutex_unlock(&pool_mutex);
            return conn;
        } else if (!connection_pool[i].connection && free_index == -1) {
            free_index = i;
        }
    }
    
    // If no idle connection was found, create a new one if there's space
    if (free_index != -1) {
        conn = create_connection();
        if (conn) {
            connection_pool[free_index].connection = conn;
            connection_pool[free_index].in_use = true;
            connection_pool[free_index].last_used = time(NULL);
            writeDebugLog("Created new connection at index %d", free_index);
        } else {
            writeErrorLog("Failed to create new database connection");
        }
    } else {
        writeWarningLog("No available connections in the pool");
    }
    
    pthread_mutex_unlock(&pool_mutex);
    return conn;
}

/**
 * Release a connection back to the pool
 * @param conn Connection handle to release
 */
void db_release_connection(MYSQL* conn) {
    if (!conn) return;
    
    pthread_mutex_lock(&pool_mutex);
    
    // Find the connection in the pool
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection == conn) {
            connection_pool[i].in_use = false;
            connection_pool[i].last_used = time(NULL);
            writeDebugLog("Released connection %d back to pool", i);
            break;
        }
    }
    
    // Clean up idle connections
    cleanup_idle_connections();
    
    pthread_mutex_unlock(&pool_mutex);
}

/**
 * Execute a simple SQL query with no result set
 * @param query SQL query to execute
 * @return true if successful, false otherwise
 */
bool db_execute_query(const char* query) {
    if (!query) return false;
    
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get connection for executing query");
        return false;
    }
    
    writeDebugLog("Executing query: %s", query);
    int result = mysql_query(conn, query);
    
    if (result != 0) {
        writeErrorLog("MySQL query error: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return (result == 0);
}

/**
 * Execute a SQL query that returns a result set
 * @param query SQL query to execute
 * @param callback Function to call for each row in the result
 * @param user_data User data to pass to the callback
 * @return true if successful, false otherwise
 */
bool db_execute_select(const char* query, 
                       void (*callback)(MYSQL_ROW row, void* user_data), 
                       void* user_data) {
    if (!query || !callback) return false;
    
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get connection for executing select query");
        return false;
    }
    
    writeDebugLog("Executing select query: %s", query);
    int result = mysql_query(conn, query);
    
    if (result != 0) {
        writeErrorLog("MySQL query error: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        writeErrorLog("Failed to store MySQL result: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Process each row in the result set
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        callback(row, user_data);
    }
    
    mysql_free_result(res);
    db_release_connection(conn);
    
    return true;
}

/**
 * Check if database connection is active
 * @return true if connected, false otherwise
 */
bool db_is_connected(void) {
    MYSQL* conn = db_get_connection();
    bool connected = (conn != NULL);
    
    if (connected) {
        db_release_connection(conn);
    }
    
    return connected;
}

/**
 * Get the number of active connections in the pool
 * @return Number of active connections
 */
int db_get_active_connections(void) {
    int count = 0;
    
    pthread_mutex_lock(&pool_mutex);
    
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection && connection_pool[i].in_use) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&pool_mutex);
    return count;
}

/**
 * Get the last error message from the database
 * @return Error message string
 */
const char* db_get_error(void) {
    static char error_buffer[256] = {0};
    MYSQL* conn = db_get_connection();
    
    if (!conn) {
        return "Cannot connect to database";
    }
    
    strncpy(error_buffer, mysql_error(conn), sizeof(error_buffer) - 1);
    db_release_connection(conn);
    
    return error_buffer;
}

/**
 * Prepare and execute a parameterized query using prepared statements
 * @param query SQL query template with ? placeholders
 * @param bind_count Number of parameters to bind
 * @param ... Variable arguments for parameters
 * @return true if successful, false otherwise
 */
bool db_prepared_query(const char* query, int bind_count, ...) {
    if (!query || bind_count < 0) return false;
    
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get connection for prepared statement");
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        writeErrorLog("Failed to initialize statement: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        writeErrorLog("Failed to prepare statement: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        db_release_connection(conn);
        return false;
    }
    
    // Make sure the number of parameters matches
    if (mysql_stmt_param_count(stmt) != bind_count) {
        writeErrorLog("Parameter count mismatch: expected %lu, got %d", 
                   mysql_stmt_param_count(stmt), bind_count);
        mysql_stmt_close(stmt);
        db_release_connection(conn);
        return false;
    }
    
    // Only bind parameters if we have any
    bool success = true;
    if (bind_count > 0) {
        MYSQL_BIND* binds = calloc(bind_count, sizeof(MYSQL_BIND));
        if (!binds) {
            writeErrorLog("Failed to allocate memory for parameter bindings");
            mysql_stmt_close(stmt);
            db_release_connection(conn);
            return false;
        }
        
        va_list args;
        va_start(args, bind_count);
        
        // Set up the bindings based on variable arguments
        for (int i = 0; i < bind_count; i++) {
            int param_type = va_arg(args, int);
            void* param_value = va_arg(args, void*);
            
            binds[i].buffer_type = param_type;
            binds[i].buffer = param_value;
            
            // Handle strings differently
            if (param_type == MYSQL_TYPE_STRING || 
                param_type == MYSQL_TYPE_VAR_STRING) {
                binds[i].buffer_length = strlen((char*)param_value);
            }
        }
        
        va_end(args);
        
        // Bind the parameters
        if (mysql_stmt_bind_param(stmt, binds)) {
            writeErrorLog("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            success = false;
        } else {
            // Execute the prepared statement
            if (mysql_stmt_execute(stmt)) {
                writeErrorLog("Failed to execute statement: %s", mysql_stmt_error(stmt));
                success = false;
            }
        }
        
        free(binds);
    } else {
        // Execute without parameters
        if (mysql_stmt_execute(stmt)) {
            writeErrorLog("Failed to execute statement: %s", mysql_stmt_error(stmt));
            success = false;
        }
    }
    
    mysql_stmt_close(stmt);
    db_release_connection(conn);
    
    return success;
}

/**
 * Begin a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_begin_transaction(MYSQL* conn) {
    if (!conn) return false;
    
    int result = mysql_query(conn, "START TRANSACTION");
    if (result != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        return false;
    }
    
    return true;
}

/**
 * Commit a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_commit_transaction(MYSQL* conn) {
    if (!conn) return false;
    
    int result = mysql_query(conn, "COMMIT");
    if (result != 0) {
        writeErrorLog("Failed to commit transaction: %s", mysql_error(conn));
        return false;
    }
    
    return true;
}

/**
 * Rollback a transaction
 * @param conn MySQL connection
 * @return true if successful, false otherwise
 */
bool db_rollback_transaction(MYSQL* conn) {
    if (!conn) return false;
    
    int result = mysql_query(conn, "ROLLBACK");
    if (result != 0) {
        writeErrorLog("Failed to rollback transaction: %s", mysql_error(conn));
        return false;
    }
    
    return true;
}

/**
 * Escape a string to prevent SQL injection
 * @param input Input string to escape
 * @param output Output buffer for escaped string
 * @param size Size of output buffer
 * @return true if successful, false otherwise
 */
bool db_escape_string(const char* input, char* output, size_t size) {
    if (!input || !output || size == 0) return false;
    
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get connection for escaping string");
        return false;
    }
    
    // Reserve space for null terminator
    size_t max_escaped_size = (size - 1) * 2;  // MySQL docs: escaped string can be up to 2*original_size+1
    char* temp_buffer = malloc(max_escaped_size + 1);
    
    if (!temp_buffer) {
        writeErrorLog("Failed to allocate memory for escaping string");
        db_release_connection(conn);
        return false;
    }
    
    // Escape the string
    mysql_real_escape_string(conn, temp_buffer, input, strlen(input));
    
    // Copy to output buffer with size checking
    strncpy(output, temp_buffer, size - 1);
    output[size - 1] = '\0';
    
    free(temp_buffer);
    db_release_connection(conn);
    
    return true;
}

/**************************** Internal Functions ****************************/

/**
 * Create a new database connection
 * @return MySQL connection handle or NULL on failure
 */
static MYSQL* create_connection(void) {
    MYSQL* conn = mysql_init(NULL);
    
    if (!conn) {
        writeErrorLog("Failed to initialize MySQL connection object");
        return NULL;
    }
    
    // Set connection timeout
    unsigned int timeout = DB_CONNECT_TIMEOUT;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    
    // Try to connect to the database
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, 
                           DB_NAME, DB_PORT, NULL, 0)) {
        writeErrorLog("Failed to connect to database: %s", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    
    // Set character set to UTF8
    if (mysql_set_character_set(conn, "utf8")) {
        writeWarningLog("Failed to set character set: %s", mysql_error(conn));
    }
    
    return conn;
}

/**
 * Close a database connection
 * @param conn MySQL connection to close
 */
static void close_connection(MYSQL* conn) {
    if (conn) {
        mysql_close(conn);
    }
}

/**
 * Clean up idle connections that have been unused for too long
 */
static void cleanup_idle_connections(void) {
    time_t current_time = time(NULL);
    
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection && !connection_pool[i].in_use &&
            (current_time - connection_pool[i].last_used > CONNECTION_IDLE_TIMEOUT)) {
            writeDebugLog("Closing idle connection %d (unused for %ld seconds)",
                       i, current_time - connection_pool[i].last_used);
            mysql_close(connection_pool[i].connection);
            connection_pool[i].connection = NULL;
            connection_pool[i].last_used = 0;
        }
    }
}