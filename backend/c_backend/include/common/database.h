/**
 * @file database.h
 * @brief Database interface for the ATM Management System
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <stdbool.h>

/**
 * @brief Initialize database connection
 * @return MYSQL* Database connection handle or NULL on failure
 */
MYSQL* db_connect(void);

/**
 * @brief Close database connection
 * @param conn Database connection handle
 */
void db_disconnect(MYSQL *conn);

/**
 * @brief Execute a SQL query
 * @param conn Database connection handle
 * @param query SQL query to execute
 * @return MYSQL_RES* Result set or NULL on failure
 */
MYSQL_RES* db_query(MYSQL *conn, const char *query);

/**
 * @brief Execute a SQL query with no result set
 * @param conn Database connection handle
 * @param query SQL query to execute
 * @return bool True on success, false on failure
 */
bool db_execute(MYSQL *conn, const char *query);

/**
 * @brief Get the last error message
 * @param conn Database connection handle
 * @return const char* Error message
 */
const char* db_error(MYSQL *conn);

/**
 * @brief Get the number of affected rows
 * @param conn Database connection handle
 * @return unsigned long long Number of affected rows
 */
unsigned long long db_affected_rows(MYSQL *conn);

/**
 * @brief Get the last insert ID
 * @param conn Database connection handle
 * @return unsigned long long Last insert ID
 */
unsigned long long db_last_insert_id(MYSQL *conn);

/**
 * @brief Escape a string for use in a SQL query
 * @param conn Database connection handle
 * @param to Destination buffer
 * @param from Source string
 * @param length Length of source string
 * @return unsigned long Length of escaped string
 */
unsigned long db_escape_string(MYSQL *conn, char *to, const char *from, unsigned long length);

/**
 * @brief Begin a transaction
 * @param conn Database connection handle
 * @return bool True on success, false on failure
 */
bool db_begin_transaction(MYSQL *conn);

/**
 * @brief Commit a transaction
 * @param conn Database connection handle
 * @return bool True on success, false on failure
 */
bool db_commit(MYSQL *conn);

/**
 * @brief Rollback a transaction
 * @param conn Database connection handle
 * @return bool True on success, false on failure
 */
bool db_rollback(MYSQL *conn);

#endif /* DATABASE_H */
