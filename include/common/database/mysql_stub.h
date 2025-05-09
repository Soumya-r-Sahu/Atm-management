/**
 * @file mysql_stub.h
 * @brief Stub implementation for MySQL when not available
 */

#ifndef MYSQL_STUB_H
#define MYSQL_STUB_H

#ifdef NO_MYSQL

#include <stddef.h>

// MySQL type definitions for stub implementation
typedef struct MYSQL MYSQL;
typedef struct MYSQL_RES MYSQL_RES;
typedef char** MYSQL_ROW;  // MYSQL_ROW is a pointer to array of strings
typedef struct MYSQL_FIELD MYSQL_FIELD;

// Stub function declarations
MYSQL *mysql_init(MYSQL *mysql);
MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, 
                         const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag);
int mysql_query(MYSQL *mysql, const char *query);
MYSQL_RES *mysql_store_result(MYSQL *mysql);
MYSQL_ROW mysql_fetch_row(MYSQL_RES *result);
unsigned int mysql_num_fields(MYSQL_RES *result);
unsigned long mysql_num_rows(MYSQL_RES *result);
void mysql_free_result(MYSQL_RES *result);
void mysql_close(MYSQL *mysql);
const char *mysql_error(MYSQL *mysql);

#endif // NO_MYSQL

#endif // MYSQL_STUB_H
