#include "../../include/common/database/db_config.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/paths.h"
#include "../../include/common/database/transaction_manager.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

// Connection pool structures
static DatabaseConnection connection_pool[MAX_DB_CONNECTIONS];
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool pool_initialized = false;

// Initialize the database connection pool
bool db_init(void) {
    pthread_mutex_lock(&pool_mutex);
    
    if (pool_initialized) {
        pthread_mutex_unlock(&pool_mutex);
        return true;
    }
    
    // Initialize all connections
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        connection_pool[i].connection = NULL;
        connection_pool[i].in_use = false;
        connection_pool[i].last_used = 0;
    }
    
    writeInfoLog("Database connection pool initialized with %d connections", MAX_DB_CONNECTIONS);
    pool_initialized = true;
    
    pthread_mutex_unlock(&pool_mutex);
    return true;
}

// Clean up the database connection pool
void db_cleanup(void) {
    pthread_mutex_lock(&pool_mutex);
    
    // Close all connections
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection != NULL) {
            mysql_close(connection_pool[i].connection);
            connection_pool[i].connection = NULL;
            connection_pool[i].in_use = false;
        }
    }
    
    writeInfoLog("Database connection pool cleaned up");
    pool_initialized = false;
    
    pthread_mutex_unlock(&pool_mutex);
}

// Get a connection from the pool
MYSQL* db_get_connection(void) {
    if (!pool_initialized) {
        if (!db_init()) {
            writeErrorLog("Failed to initialize database pool");
            return NULL;
        }
    }
    
    pthread_mutex_lock(&pool_mutex);
    
    // First, try to find an existing connection
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection != NULL && !connection_pool[i].in_use) {
            // Check if connection is still valid
            if (mysql_ping(connection_pool[i].connection) == 0) {
                connection_pool[i].in_use = true;
                connection_pool[i].last_used = time(NULL);
                pthread_mutex_unlock(&pool_mutex);
                return connection_pool[i].connection;
            } else {
                // Connection is dead, close and mark as null
                mysql_close(connection_pool[i].connection);
                connection_pool[i].connection = NULL;
            }
        }
    }
    
    // No existing connections available, try to create a new one
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection == NULL) {
            MYSQL* conn = mysql_init(NULL);
            
            if (conn == NULL) {
                writeErrorLog("Failed to initialize MySQL connection");
                pthread_mutex_unlock(&pool_mutex);
                return NULL;
            }
            
            // Set options
            unsigned int timeout = CONNECTION_TIMEOUT;
            mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
            
            // Connect to database
            if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
                connection_pool[i].connection = conn;
                connection_pool[i].in_use = true;
                connection_pool[i].last_used = time(NULL);
                pthread_mutex_unlock(&pool_mutex);
                return conn;
            } else {
                writeErrorLog("Failed to connect to MySQL: %s", mysql_error(conn));
                mysql_close(conn);
                pthread_mutex_unlock(&pool_mutex);
                return NULL;
            }
        }
    }
    
    // If we get here, no connections are available
    writeErrorLog("No database connections available in the pool");
    pthread_mutex_unlock(&pool_mutex);
    return NULL;
}

// Release a connection back to the pool
void db_release_connection(MYSQL* conn) {
    if (conn == NULL) {
        return;
    }
    
    pthread_mutex_lock(&pool_mutex);
    
    // Find the connection in the pool
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connection_pool[i].connection == conn) {
            connection_pool[i].in_use = false;
            connection_pool[i].last_used = time(NULL);
            pthread_mutex_unlock(&pool_mutex);
            return;
        }
    }
    
    // If we get here, the connection wasn't in our pool
    writeWarningLog("Attempted to release a connection not in the pool");
    pthread_mutex_unlock(&pool_mutex);
}

// Execute a simple SQL query with no result set
bool db_execute_query(const char* query) {
    MYSQL* conn = db_get_connection();
    if (!conn) {
        return false;
    }
    
    bool result = (mysql_query(conn, query) == 0);
    
    if (!result) {
        writeErrorLog("SQL Error: %s\nQuery: %s", mysql_error(conn), query);
    }
    
    db_release_connection(conn);
    return result;
}

// Execute a SQL query that returns a result set
bool db_execute_select(const char* query, void (*callback)(MYSQL_ROW row, void* user_data), void* user_data) {
    MYSQL* conn = db_get_connection();
    if (!conn) {
        return false;
    }
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                callback(row, user_data);
            }
            mysql_free_result(result);
            success = true;
        } else {
            writeErrorLog("Failed to store result: %s", mysql_error(conn));
        }
    } else {
        writeErrorLog("SQL Error: %s\nQuery: %s", mysql_error(conn), query);
    }
    
    db_release_connection(conn);
    return success;
}

// Check if a customer exists
bool doesCustomerExist(const char* customerId) {
    if (!customerId) {
        writeErrorLog("NULL customer ID provided to doesCustomerExist");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in doesCustomerExist");
        return false;
    }
    
    char query[200];
    sprintf(query, "SELECT COUNT(*) FROM %s WHERE %s = '%s'", 
            TABLE_CUSTOMERS, COL_CUSTOMER_ID, customerId);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                exists = (atoi(row[0]) > 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in doesCustomerExist: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return exists;
}

// Check if an account exists
bool doesAccountExist(const char* accountNumber) {
    if (!accountNumber) {
        writeErrorLog("NULL account number provided to doesAccountExist");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in doesAccountExist");
        return false;
    }
    
    char query[200];
    sprintf(query, "SELECT COUNT(*) FROM %s WHERE %s = '%s'", 
            TABLE_ACCOUNTS, COL_ACCOUNT_NUMBER, accountNumber);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                exists = (atoi(row[0]) > 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in doesAccountExist: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return exists;
}

// Check if a card exists
bool doesCardExist(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in doesCardExist");
        return false;
    }
    
    char query[200];
    sprintf(query, "SELECT COUNT(*) FROM %s WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                exists = (atoi(row[0]) > 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in doesCardExist: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return exists;
}

// Get account balance
double getAccountBalance(const char* accountNumber) {
    if (!accountNumber) {
        writeErrorLog("NULL account number provided to getAccountBalance");
        return -1.0;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getAccountBalance");
        return -1.0;
    }
    
    char query[200];
    sprintf(query, "SELECT %s FROM %s WHERE %s = '%s'", 
            COL_BALANCE, TABLE_ACCOUNTS, COL_ACCOUNT_NUMBER, accountNumber);
    
    double balance = -1.0;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                balance = atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getAccountBalance: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return balance;
}

// Update account balance
bool updateAccountBalance(const char* accountNumber, double newBalance) {
    if (!accountNumber) {
        writeErrorLog("NULL account number provided to updateAccountBalance");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in updateAccountBalance");
        return false;
    }
    
    char query[200];
    sprintf(query, "UPDATE %s SET %s = %.2f, last_transaction = NOW() WHERE %s = '%s'", 
            TABLE_ACCOUNTS, COL_BALANCE, newBalance, COL_ACCOUNT_NUMBER, accountNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            writeInfoLog("Updated balance for account %s to %.2f", accountNumber, newBalance);
        } else {
            writeErrorLog("No rows affected when updating balance for account %s", accountNumber);
        }
    } else {
        writeErrorLog("MySQL query error in updateAccountBalance: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Check if a card is active
bool isCardActive(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in isCardActive");
        return false;
    }
    
    char query[200];
    sprintf(query, "SELECT status FROM %s WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool active = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                active = (strcmp(row[0], "ACTIVE") == 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in isCardActive: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return active;
}

// Get account number associated with a card
char* getAccountNumberForCard(int cardNumber, char* accountNumber, size_t size) {
    if (!accountNumber || size == 0) {
        writeErrorLog("Invalid output buffer in getAccountNumberForCard");
        return NULL;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getAccountNumberForCard");
        return NULL;
    }
    
    char query[200];
    sprintf(query, "SELECT account_id FROM %s WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    char* result = NULL;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row && row[0]) {
                strncpy(accountNumber, row[0], size - 1);
                accountNumber[size - 1] = '\0';
                result = accountNumber;
            }
            mysql_free_result(res);
        }
    } else {
        writeErrorLog("MySQL query error in getAccountNumberForCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return result;
}

// Verify PIN for a card
bool verifyCardPin(int cardNumber, const char* pinHash) {
    if (!pinHash) {
        writeErrorLog("NULL PIN hash provided to verifyCardPin");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in verifyCardPin");
        return false;
    }
    
    char query[300];
    sprintf(query, "SELECT COUNT(*) FROM %s WHERE %s = '%d' AND pin_hash = '%s'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber, pinHash);
    
    bool verified = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                verified = (atoi(row[0]) > 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in verifyCardPin: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return verified;
}

// Update PIN for a card
bool updateCardPin(int cardNumber, const char* newPinHash) {
    if (!newPinHash) {
        writeErrorLog("NULL PIN hash provided to updateCardPin");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in updateCardPin");
        return false;
    }
    
    char query[300];
    sprintf(query, "UPDATE %s SET pin_hash = '%s' WHERE %s = '%d'", 
            TABLE_CARDS, newPinHash, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[400];
            sprintf(audit_query, 
                "INSERT INTO %s (action, entity_type, entity_id, details) "
                "VALUES ('PIN_CHANGE', 'CARD', '%d', 'PIN changed by user')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log PIN change in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("PIN updated for card %d", cardNumber);
        } else {
            writeErrorLog("No rows affected when updating PIN for card %d", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in updateCardPin: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Record a transaction
bool recordTransaction(const char* transactionId, int cardNumber, const char* accountNumber, 
                       const char* type, double amount, double balanceBefore, 
                       double balanceAfter, const char* remarks) {
    if (!transactionId || !accountNumber || !type || !remarks) {
        writeErrorLog("NULL parameters provided to recordTransaction");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in recordTransaction");
        return false;
    }
    
    char query[600];
    sprintf(query, 
        "INSERT INTO %s (transaction_id, card_number, account_number, transaction_type, "
        "amount, balance_before, balance_after, status, remarks) "
        "VALUES ('%s', '%d', '%s', '%s', %.2f, %.2f, %.2f, 'SUCCESS', '%s')",
        TABLE_TRANSACTIONS, transactionId, cardNumber, accountNumber, type, 
        amount, balanceBefore, balanceAfter, remarks);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        success = true;
        writeInfoLog("Transaction recorded: %s, Card: %d, Account: %s, Type: %s, Amount: %.2f", 
                   transactionId, cardNumber, accountNumber, type, amount);
    } else {
        writeErrorLog("MySQL query error in recordTransaction: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Get recent transactions for an account
bool getRecentTransactions(const char* accountNumber, int count, 
                           void (*callback)(const char*, int, const char*, const char*, 
                                           double, double, double, const char*, const char*, void*), 
                           void* userData) {
    if (!accountNumber || count <= 0 || !callback) {
        writeErrorLog("Invalid parameters provided to getRecentTransactions");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getRecentTransactions");
        return false;
    }
    
    char query[400];
    sprintf(query, 
        "SELECT transaction_id, card_number, account_number, transaction_type, "
        "amount, balance_before, balance_after, transaction_date, remarks "
        "FROM %s WHERE account_number = '%s' "
        "ORDER BY transaction_date DESC LIMIT %d",
        TABLE_TRANSACTIONS, accountNumber, count);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                callback(
                    row[0],                  // transaction_id
                    atoi(row[1]),            // card_number
                    row[2],                  // account_number
                    row[3],                  // transaction_type
                    atof(row[4]),            // amount
                    atof(row[5]),            // balance_before
                    atof(row[6]),            // balance_after
                    row[7],                  // transaction_date
                    row[8],                  // remarks
                    userData
                );
            }
            mysql_free_result(result);
            success = true;
        }
    } else {
        writeErrorLog("MySQL query error in getRecentTransactions: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Track daily withdrawal
bool trackDailyWithdrawal(int cardNumber, double amount) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in trackDailyWithdrawal");
        return false;
    }
    
    char query[300];
    sprintf(query, 
        "INSERT INTO %s (card_number, amount, withdrawal_date) "
        "VALUES ('%d', %.2f, CURDATE())",
        TABLE_DAILY_WITHDRAWALS, cardNumber, amount);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        success = true;
        writeInfoLog("Daily withdrawal tracked: Card %d, Amount %.2f", cardNumber, amount);
    } else {
        writeErrorLog("MySQL query error in trackDailyWithdrawal: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Get total daily withdrawals for a card
double getDailyWithdrawalTotal(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getDailyWithdrawalTotal");
        return -1.0;
    }
    
    char query[300];
    sprintf(query, 
        "SELECT COALESCE(SUM(amount), 0) FROM %s "
        "WHERE card_number = '%d' AND withdrawal_date = CURDATE()",
        TABLE_DAILY_WITHDRAWALS, cardNumber);
    
    double total = -1.0;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                total = atof(row[0]);
            } else {
                total = 0.0;  // No withdrawals today
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getDailyWithdrawalTotal: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return total;
}

// Get daily withdrawal limit for a card
double getDailyWithdrawalLimit(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getDailyWithdrawalLimit");
        return -1.0;
    }
    
    char query[200];
    sprintf(query, "SELECT daily_limit FROM %s WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    double limit = -1.0;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                limit = atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getDailyWithdrawalLimit: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return limit;
}

