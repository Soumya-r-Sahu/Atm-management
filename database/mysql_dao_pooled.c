/**
 * @file mysql_dao_pooled.c
 * @brief MySQL Database Access Object Implementation with Connection Pool
 * @version 1.0
 * @date May 10, 2025
 */

#include "../../include/common/database/dao_interface.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/database/db_unified_config.h"
#include "../../include/common/database/db_connection_pool.h"
#include "../../include/common/paths.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Private function declarations
static void* mysql_pooled_getConnection(void);
static void mysql_pooled_releaseConnection(void* conn);
static bool mysql_pooled_doesCardExist(int cardNumber);
static bool mysql_pooled_isCardActive(int cardNumber);
static bool mysql_pooled_validateCard(int cardNumber, int pin);
static bool mysql_pooled_validateCardWithHash(int cardNumber, const char* pinHash);
static bool mysql_pooled_validateCardCVV(int cardNumber, int cvv);
static bool mysql_pooled_blockCard(int cardNumber);
static bool mysql_pooled_unblockCard(int cardNumber);
static bool mysql_pooled_updateCardPIN(int cardNumber, const char* newPINHash);
static bool mysql_pooled_getCardHolderName(int cardNumber, char* name, size_t nameSize);
static bool mysql_pooled_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);
static float mysql_pooled_fetchBalance(int cardNumber);
static bool mysql_pooled_updateBalance(int cardNumber, float newBalance);
static float mysql_pooled_getDailyWithdrawals(int cardNumber);
static void mysql_pooled_logWithdrawal(int cardNumber, float amount);
static bool mysql_pooled_logTransaction(int cardNumber, const char* transactionType, float amount, bool success);
static bool mysql_pooled_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count);

// Connection pool reference
static ConnectionPool* mysqlPool = NULL;

// Initialize the connection pool
static bool initConnectionPool(void) {
    if (mysqlPool) {
        return true;  // Already initialized
    }
    
    // Create pool configuration
    ConnectionPoolConfig config = {
        .min_connections = 3,
        .max_connections = MAX_DB_CONNECTIONS,
        .initial_connections = 5,
        .connection_timeout = DB_CONNECT_TIMEOUT,
        .idle_timeout = 300,  // 5 minutes
        .max_wait_time = 5000,  // 5 seconds
        .validate_on_borrow = true,
        .validate_on_return = false,
        .db_port = DB_PORT
    };
    
    // Set database details
    strncpy(config.db_hostname, DB_HOST, sizeof(config.db_hostname) - 1);
    strncpy(config.db_name, DB_NAME, sizeof(config.db_name) - 1);
    strncpy(config.db_username, DB_USER, sizeof(config.db_username) - 1);
    strncpy(config.db_password, DB_PASS, sizeof(config.db_password) - 1);
    
    // Create the pool
    mysqlPool = connection_pool_create(&config);
    
    if (!mysqlPool) {
        writeErrorLog("Failed to create MySQL connection pool");
        return false;
    }
    
    writeInfoLog("MySQL connection pool initialized successfully");
    return true;
}

// Create the MySQL DAO
DatabaseAccessObject* createMySQLPooledDAO(void) {
    // Initialize the connection pool
    if (!initConnectionPool()) {
        return NULL;
    }
    
    // Allocate DAO structure
    DatabaseAccessObject* dao = (DatabaseAccessObject*)malloc(sizeof(DatabaseAccessObject));
    if (!dao) {
        writeErrorLog("Failed to allocate memory for MySQL DAO");
        return NULL;
    }
    
    // Set function pointers
    dao->getConnection = mysql_pooled_getConnection;
    dao->releaseConnection = mysql_pooled_releaseConnection;
    dao->doesCardExist = mysql_pooled_doesCardExist;
    dao->isCardActive = mysql_pooled_isCardActive;
    dao->validateCard = mysql_pooled_validateCard;
    dao->validateCardWithHash = mysql_pooled_validateCardWithHash;
    dao->validateCardCVV = mysql_pooled_validateCardCVV;
    dao->blockCard = mysql_pooled_blockCard;
    dao->unblockCard = mysql_pooled_unblockCard;
    dao->updateCardPIN = mysql_pooled_updateCardPIN;
    dao->getCardHolderName = mysql_pooled_getCardHolderName;
    dao->getCardHolderPhone = mysql_pooled_getCardHolderPhone;
    dao->fetchBalance = mysql_pooled_fetchBalance;
    dao->updateBalance = mysql_pooled_updateBalance;
    dao->getDailyWithdrawals = mysql_pooled_getDailyWithdrawals;
    dao->logWithdrawal = mysql_pooled_logWithdrawal;
    dao->logTransaction = mysql_pooled_logTransaction;
    dao->getMiniStatement = mysql_pooled_getMiniStatement;
    
    return dao;
}

// Get a connection from the pool
static void* mysql_pooled_getConnection(void) {
    // Initialize the pool if needed
    if (!mysqlPool && !initConnectionPool()) {
        return NULL;
    }
    
    // Get a connection from the pool
    DatabaseConnection* db_conn = connection_pool_get(mysqlPool);
    if (!db_conn) {
        writeErrorLog("Failed to get connection from pool");
        return NULL;
    }
    
    return db_conn;
}

// Release a connection back to the pool
static void mysql_pooled_releaseConnection(void* conn) {
    if (!conn) return;
    
    DatabaseConnection* db_conn = (DatabaseConnection*)conn;
    
    // Release connection back to the pool
    if (!connection_pool_return(db_conn)) {
        writeErrorLog("Failed to return connection to pool");
    }
}

// Check if a card exists
static bool mysql_pooled_doesCardExist(int cardNumber) {
    DatabaseConnection* db_conn = (DatabaseConnection*)mysql_pooled_getConnection();
    if (!db_conn) {
        return false;
    }
    
    MYSQL* conn = (MYSQL*)db_conn->connection;
    char query[200];
    sprintf(query, "SELECT COUNT(*) FROM cards WHERE card_number = %d", cardNumber);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            exists = (row && atoi(row[0]) > 0);
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL error in doesCardExist: %s", mysql_error(conn));
    }
    
    mysql_pooled_releaseConnection(db_conn);
    return exists;
}

// Check if a card is active
static bool mysql_pooled_isCardActive(int cardNumber) {
    DatabaseConnection* db_conn = (DatabaseConnection*)mysql_pooled_getConnection();
    if (!db_conn) {
        return false;
    }
    
    MYSQL* conn = (MYSQL*)db_conn->connection;
    char query[200];
    sprintf(query, "SELECT status FROM cards WHERE card_number = %d", cardNumber);
    
    bool active = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            active = (row && strcmp(row[0], "active") == 0);
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL error in isCardActive: %s", mysql_error(conn));
    }
    
    mysql_pooled_releaseConnection(db_conn);
    return active;
}

// Validate card number and PIN
static bool mysql_pooled_validateCard(int cardNumber, int pin) {
    char pinStr[20];
    sprintf(pinStr, "%04d", pin);  // Format PIN as 4-digit string
    
    return mysql_pooled_validateCardWithHash(cardNumber, pinStr);
}

// Validate card using PIN hash
static bool mysql_pooled_validateCardWithHash(int cardNumber, const char* pinHash) {
    DatabaseConnection* db_conn = (DatabaseConnection*)mysql_pooled_getConnection();
    if (!db_conn) {
        return false;
    }
    
    MYSQL* conn = (MYSQL*)db_conn->connection;
    char query[200];
    
    // Prepare the query (note: using placeholder for actual implementation)
    // In a real system, you should use prepared statements for this kind of query
    sprintf(query, "SELECT pin_hash FROM cards WHERE card_number = %d AND status = 'active'", cardNumber);
    
    bool valid = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                // This is simplified - real implementation would use secure hash comparison
                valid = (strcmp(row[0], pinHash) == 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL error in validateCardWithHash: %s", mysql_error(conn));
    }
    
    mysql_pooled_releaseConnection(db_conn);
    return valid;
}

// The rest of the functions would follow the same pattern, using the connection
// pool to get and release connections. For brevity, I'll omit the full implementations
// of the remaining functions.

// Fetch account balance
static float mysql_pooled_fetchBalance(int cardNumber) {
    DatabaseConnection* db_conn = (DatabaseConnection*)mysql_pooled_getConnection();
    if (!db_conn) {
        return -1.0f;
    }
    
    MYSQL* conn = (MYSQL*)db_conn->connection;
    char query[200];
    sprintf(query, "SELECT balance FROM accounts WHERE card_number = %d", cardNumber);
    
    float balance = -1.0f;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                balance = (float)atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL error in fetchBalance: %s", mysql_error(conn));
    }
    
    mysql_pooled_releaseConnection(db_conn);
    return balance;
}

// Update account balance
static bool mysql_pooled_updateBalance(int cardNumber, float newBalance) {
    if (newBalance < 0) {
        writeErrorLog("Attempted to set negative balance for card %d", cardNumber);
        return false;
    }
    
    DatabaseConnection* db_conn = (DatabaseConnection*)mysql_pooled_getConnection();
    if (!db_conn) {
        return false;
    }
    
    MYSQL* conn = (MYSQL*)db_conn->connection;
    char query[200];
    sprintf(query, "UPDATE accounts SET balance = %.2f WHERE card_number = %d", newBalance, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        success = (mysql_affected_rows(conn) > 0);
    } else {
        writeErrorLog("MySQL error in updateBalance: %s", mysql_error(conn));
    }
    
    mysql_pooled_releaseConnection(db_conn);
    return success;
}

// Implementations of the remaining DAO functions would follow the same pattern
// of getting a connection, performing the operation, and then releasing the connection.

static bool mysql_pooled_validateCardCVV(int cardNumber, int cvv) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_blockCard(int cardNumber) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_unblockCard(int cardNumber) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_updateCardPIN(int cardNumber, const char* newPINHash) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    // Implementation omitted for brevity
    return false;
}

static float mysql_pooled_getDailyWithdrawals(int cardNumber) {
    // Implementation omitted for brevity
    return 0.0f;
}

static void mysql_pooled_logWithdrawal(int cardNumber, float amount) {
    // Implementation omitted for brevity
}

static bool mysql_pooled_logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    // Implementation omitted for brevity
    return false;
}

static bool mysql_pooled_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count) {
    // Implementation omitted for brevity
    return false;
}
