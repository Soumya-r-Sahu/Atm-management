#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

// Connection pool
typedef struct {
    MYSQL** connections;
    bool* in_use;
    int size;
    pthread_mutex_t mutex;
} ConnectionPool;

static ConnectionPool pool = {NULL, NULL, 0, PTHREAD_MUTEX_INITIALIZER};

/**
 * Initialize the database connection pool
 */
bool db_init_pool(int max_connections) {
    pthread_mutex_lock(&pool.mutex);
    
    // Check if already initialized
    if (pool.connections != NULL) {
        pthread_mutex_unlock(&pool.mutex);
        writeInfoLog("Connection pool already initialized");
        return true;
    }
    
    // Allocate memory for connections
    pool.connections = (MYSQL**)malloc(max_connections * sizeof(MYSQL*));
    if (!pool.connections) {
        pthread_mutex_unlock(&pool.mutex);
        writeErrorLog("Failed to allocate memory for connection pool");
        return false;
    }
    
    // Allocate memory for in_use flags
    pool.in_use = (bool*)calloc(max_connections, sizeof(bool));
    if (!pool.in_use) {
        free(pool.connections);
        pool.connections = NULL;
        pthread_mutex_unlock(&pool.mutex);
        writeErrorLog("Failed to allocate memory for connection pool usage flags");
        return false;
    }
    
    // Set pool size
    pool.size = max_connections;
    
    // Initialize all connections
    bool success = true;
    for (int i = 0; i < max_connections; i++) {
        pool.connections[i] = mysql_init(NULL);
        if (!pool.connections[i]) {
            writeErrorLog("Failed to initialize MySQL connection %d", i);
            success = false;
            break;
        }
        
        // Connect
        if (!mysql_real_connect(pool.connections[i], 
                              DB_HOST, 
                              DB_USER, 
                              DB_PASS, 
                              DB_NAME, 
                              DB_PORT, 
                              NULL, 
                              0)) {
            writeErrorLog("Failed to connect to database for connection %d: %s", 
                        i, mysql_error(pool.connections[i]));
            mysql_close(pool.connections[i]);
            pool.connections[i] = NULL;
            success = false;
            break;
        }
        
        // Set character set
        if (mysql_set_character_set(pool.connections[i], "utf8mb4") != 0) {
            writeErrorLog("Failed to set character set for connection %d: %s", 
                        i, mysql_error(pool.connections[i]));
        }
        
        // Mark as not in use
        pool.in_use[i] = false;
    }
    
    if (!success) {
        // Clean up on failure
        for (int i = 0; i < max_connections; i++) {
            if (pool.connections[i]) {
                mysql_close(pool.connections[i]);
            }
        }
        free(pool.connections);
        pool.connections = NULL;
        free(pool.in_use);
        pool.in_use = NULL;
        pool.size = 0;
        
        pthread_mutex_unlock(&pool.mutex);
        return false;
    }
    
    writeInfoLog("Connection pool initialized with %d connections", max_connections);
    pthread_mutex_unlock(&pool.mutex);
    return true;
}

/**
 * Clean up the connection pool
 */
void db_cleanup_pool() {
    pthread_mutex_lock(&pool.mutex);
    
    if (pool.connections) {
        for (int i = 0; i < pool.size; i++) {
            if (pool.connections[i]) {
                mysql_close(pool.connections[i]);
            }
        }
        free(pool.connections);
        pool.connections = NULL;
    }
    
    if (pool.in_use) {
        free(pool.in_use);
        pool.in_use = NULL;
    }
    
    pool.size = 0;
    writeInfoLog("Connection pool cleaned up");
    pthread_mutex_unlock(&pool.mutex);
}

/**
 * Get a connection from the pool
 */
MYSQL* db_get_connection() {
    pthread_mutex_lock(&pool.mutex);
    
    // Check if pool is initialized
    if (!pool.connections || !pool.in_use || pool.size <= 0) {
        // Try to initialize with default size if not already initialized
        pthread_mutex_unlock(&pool.mutex);
        if (!db_init_pool(5)) {  // Default to 5 connections
            writeErrorLog("Connection pool not initialized and auto-init failed");
            return NULL;
        }
        pthread_mutex_lock(&pool.mutex);
    }
    
    // Find a free connection
    MYSQL* conn = NULL;
    for (int i = 0; i < pool.size; i++) {
        if (!pool.in_use[i] && pool.connections[i]) {
            // Check if connection is still alive with a simple ping
            if (mysql_ping(pool.connections[i]) != 0) {
                // Reconnect if needed
                writeInfoLog("Connection %d lost, reconnecting", i);
                mysql_close(pool.connections[i]);
                pool.connections[i] = mysql_init(NULL);
                if (!pool.connections[i]) {
                    writeErrorLog("Failed to reinitialize MySQL connection %d", i);
                    continue;
                }
                
                if (!mysql_real_connect(pool.connections[i], 
                                      DB_HOST, 
                                      DB_USER, 
                                      DB_PASS, 
                                      DB_NAME, 
                                      DB_PORT, 
                                      NULL, 
                                      0)) {
                    writeErrorLog("Failed to reconnect to database for connection %d: %s", 
                                i, mysql_error(pool.connections[i]));
                    mysql_close(pool.connections[i]);
                    pool.connections[i] = NULL;
                    continue;
                }
                
                // Set character set
                if (mysql_set_character_set(pool.connections[i], "utf8mb4") != 0) {
                    writeErrorLog("Failed to set character set for reconnected connection %d: %s", 
                                i, mysql_error(pool.connections[i]));
                }
            }
            
            // Mark as in use and return
            pool.in_use[i] = true;
            conn = pool.connections[i];
            break;
        }
    }
    
    pthread_mutex_unlock(&pool.mutex);
    
    // If we couldn't get a connection, log an error
    if (!conn) {
        writeErrorLog("No available connections in the pool");
    }
    
    return conn;
}

/**
 * Release a connection back to the pool
 */
void db_release_connection(MYSQL* conn) {
    if (!conn) {
        return;
    }
    
    pthread_mutex_lock(&pool.mutex);
    
    // Find the connection in the pool
    bool found = false;
    for (int i = 0; i < pool.size; i++) {
        if (pool.connections[i] == conn) {
            // Mark as not in use
            pool.in_use[i] = false;
            found = true;
            break;
        }
    }
    
    // If not found, it wasn't from our pool
    if (!found) {
        writeWarningLog("Attempt to release a connection not from the pool");
    }
    
    pthread_mutex_unlock(&pool.mutex);
}

/**
 * Execute a simple query with error handling
 */
bool db_execute_query(MYSQL* conn, const char* query) {
    if (!conn || !query) {
        writeErrorLog("Invalid parameters in db_execute_query");
        return false;
    }
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Query execution failed: %s - Query: %s", mysql_error(conn), query);
        return false;
    }
    
    return true;
}

/**
 * Begin a database transaction
 */
bool db_begin_transaction(MYSQL* conn) {
    return db_execute_query(conn, "START TRANSACTION");
}

/**
 * Commit a database transaction
 */
bool db_commit_transaction(MYSQL* conn) {
    return db_execute_query(conn, "COMMIT");
}

/**
 * Rollback a database transaction
 */
bool db_rollback_transaction(MYSQL* conn) {
    return db_execute_query(conn, "ROLLBACK");
}
