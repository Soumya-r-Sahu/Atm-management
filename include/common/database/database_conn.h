#ifndef DATABASE_CONN_H
#define DATABASE_CONN_H

#include <mysql/mysql.h>

/**
 * @brief Initialize the database connection pool
 * @return true if initialization was successful, false otherwise
 */
bool db_init(void);

/**
 * @brief Clean up and close all database connections
 */
void db_cleanup(void);

/**
 * @brief Get a database connection from the pool
 * @return MYSQL* connection handle or NULL if no connection is available
 */
MYSQL* db_get_connection(void);

/**
 * @brief Release a database connection back to the pool
 * @param conn The connection to release
 */
void db_release_connection(MYSQL* conn);

/**
 * @brief Execute a database transaction with multiple queries
 * @param queries Array of query strings
 * @param num_queries Number of queries in the array
 * @return true if all queries executed successfully, false otherwise
 */
bool db_execute_transaction(const char** queries, int num_queries);

#endif /* DATABASE_CONN_H */
