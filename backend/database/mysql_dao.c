/**
 * @file mysql_dao.c
 * @brief MySQL Database Access Object Implementation
 * @version 1.0
 * @date May 10, 2025
 */

#include "../../../include/common/database/dao_interface.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/common/database/db_unified_config.h"
#include "../../../include/common/paths.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Private function declarations
static void* mysql_getConnection(void);
static void mysql_releaseConnection(void* conn);
static bool mysql_doesCardExist(int cardNumber);
static bool mysql_isCardActive(int cardNumber);
static bool mysql_validateCard(int cardNumber, int pin);
static bool mysql_validateCardWithHash(int cardNumber, const char* pinHash);
static bool mysql_validateCardCVV(int cardNumber, int cvv);
static bool mysql_blockCard(int cardNumber);
static bool mysql_unblockCard(int cardNumber);
static bool mysql_updateCardPIN(int cardNumber, const char* newPINHash);
static bool mysql_getCardHolderName(int cardNumber, char* name, size_t nameSize);
static bool mysql_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);
static float mysql_fetchBalance(int cardNumber);
static bool mysql_updateBalance(int cardNumber, float newBalance);
static float mysql_getDailyWithdrawals(int cardNumber);
static void mysql_logWithdrawal(int cardNumber, float amount);
static bool mysql_logTransaction(int cardNumber, const char* transactionType, float amount, bool success);
static bool mysql_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count);

// Connection pool
static MYSQL* connectionPool[MAX_DB_CONNECTIONS];
static bool inUse[MAX_DB_CONNECTIONS];
static time_t lastUsed[MAX_DB_CONNECTIONS];
static bool poolInitialized = false;

// Mutex for thread safety
#include <pthread.h>
static pthread_mutex_t poolMutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the connection pool
static bool initConnectionPool(void) {
    pthread_mutex_lock(&poolMutex);
    
    if (poolInitialized) {
        pthread_mutex_unlock(&poolMutex);
        return true;
    }
    
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        connectionPool[i] = NULL;
        inUse[i] = false;
        lastUsed[i] = 0;
    }
    
    // Initialize MySQL library
    if (mysql_library_init(0, NULL, NULL)) {
        writeErrorLog("Failed to initialize MySQL client library");
        pthread_mutex_unlock(&poolMutex);
        return false;
    }
    
    // Create at least one connection to test the configuration
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        writeErrorLog("Failed to initialize MySQL connection structure");
        pthread_mutex_unlock(&poolMutex);
        return false;
    }
    
    // Set connection timeout
    unsigned int timeout = DB_CONNECT_TIMEOUT;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    
    // Try to connect
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)) {
        writeErrorLog("Failed to connect to MySQL: %s", mysql_error(conn));
        mysql_close(conn);
        pthread_mutex_unlock(&poolMutex);
        return false;
    }
    
    // Success - add to pool
    connectionPool[0] = conn;
    inUse[0] = false;
    lastUsed[0] = time(NULL);
    poolInitialized = true;
    
    writeInfoLog("MySQL connection pool initialized successfully");
    pthread_mutex_unlock(&poolMutex);
    return true;
}

// Clean up idle connections
static void cleanupIdleConnections(void) {
    time_t now = time(NULL);
    
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connectionPool[i] && !inUse[i] && (now - lastUsed[i] > CONNECTION_IDLE_TIMEOUT)) {
            mysql_close(connectionPool[i]);
            connectionPool[i] = NULL;
            lastUsed[i] = 0;
            writeDebugLog("Closed idle connection %d", i);
        }
    }
}

// Get a connection from the pool with O(1) best case
static void* mysql_getConnection(void) {
    // Initialize the pool if needed
    if (!poolInitialized && !initConnectionPool()) {
        return NULL;
    }
    
    pthread_mutex_lock(&poolMutex);
    
    // First, look for an existing idle connection
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connectionPool[i] && !inUse[i]) {
            // Found an idle connection
            inUse[i] = true;
            lastUsed[i] = time(NULL);
            
            // Ping to ensure connection is still alive
            if (mysql_ping(connectionPool[i]) != 0) {
                // Connection is dead, try to reconnect
                writeWarningLog("Connection %d is dead, reconnecting", i);
                mysql_close(connectionPool[i]);
                
                connectionPool[i] = mysql_init(NULL);
                if (!connectionPool[i]) {
                    inUse[i] = false;
                    pthread_mutex_unlock(&poolMutex);
                    return NULL;
                }
                
                unsigned int timeout = DB_CONNECT_TIMEOUT;
                mysql_options(connectionPool[i], MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
                
                if (!mysql_real_connect(connectionPool[i], DB_HOST, DB_USER, DB_PASS, 
                                       DB_NAME, DB_PORT, NULL, 0)) {
                    writeErrorLog("Failed to reconnect: %s", mysql_error(connectionPool[i]));
                    mysql_close(connectionPool[i]);
                    connectionPool[i] = NULL;
                    inUse[i] = false;
                    pthread_mutex_unlock(&poolMutex);
                    return NULL;
                }
            }
            
            pthread_mutex_unlock(&poolMutex);
            return connectionPool[i];
        }
    }
    
    // No idle connection found, try to create a new one
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (!connectionPool[i]) {
            // Found an empty slot
            connectionPool[i] = mysql_init(NULL);
            if (!connectionPool[i]) {
                pthread_mutex_unlock(&poolMutex);
                return NULL;
            }
            
            unsigned int timeout = DB_CONNECT_TIMEOUT;
            mysql_options(connectionPool[i], MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
            
            if (!mysql_real_connect(connectionPool[i], DB_HOST, DB_USER, DB_PASS, 
                                   DB_NAME, DB_PORT, NULL, 0)) {
                writeErrorLog("Failed to connect: %s", mysql_error(connectionPool[i]));
                mysql_close(connectionPool[i]);
                connectionPool[i] = NULL;
                pthread_mutex_unlock(&poolMutex);
                return NULL;
            }
            
            inUse[i] = true;
            lastUsed[i] = time(NULL);
            pthread_mutex_unlock(&poolMutex);
            return connectionPool[i];
        }
    }
    
    // Pool is full and all connections are in use
    writeWarningLog("Connection pool full, could not get a connection");
    pthread_mutex_unlock(&poolMutex);
    return NULL;
}

// Release a connection back to the pool with O(1) complexity
static void mysql_releaseConnection(void* conn) {
    if (!conn) return;
    
    pthread_mutex_lock(&poolMutex);
    
    for (int i = 0; i < MAX_DB_CONNECTIONS; i++) {
        if (connectionPool[i] == conn) {
            inUse[i] = false;
            lastUsed[i] = time(NULL);
            break;
        }
    }
    
    // Clean up idle connections if needed
    cleanupIdleConnections();
    
    pthread_mutex_unlock(&poolMutex);
}

// Check if a card exists in the database - O(1) with index
static bool mysql_doesCardExist(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }
    
    char query[200];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s WHERE %s = %d", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                exists = atoi(row[0]) > 0;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return exists;
}

// Check if a card is active - O(1) with index
static bool mysql_isCardActive(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }
    
    char query[200];
    snprintf(query, sizeof(query), "SELECT %s FROM %s WHERE %s = %d", 
            COL_STATUS, TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool active = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                active = (strcasecmp(row[0], "ACTIVE") == 0);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return active;
}

// Create the MySQL DAO
DatabaseAccessObject* createMySQLDAO(void) {
    DatabaseAccessObject* dao = (DatabaseAccessObject*)malloc(sizeof(DatabaseAccessObject));
    if (!dao) {
        writeErrorLog("Failed to allocate memory for MySQL DAO");
        return NULL;
    }
    
    // Initialize with MySQL implementations
    dao->doesCardExist = mysql_doesCardExist;
    dao->isCardActive = mysql_isCardActive;
    dao->validateCard = mysql_validateCard;
    dao->validateCardWithHash = mysql_validateCardWithHash;
    dao->validateCardCVV = mysql_validateCardCVV;
    dao->blockCard = mysql_blockCard;
    dao->unblockCard = mysql_unblockCard;
    dao->updateCardPIN = mysql_updateCardPIN;
    dao->getCardHolderName = mysql_getCardHolderName;
    dao->getCardHolderPhone = mysql_getCardHolderPhone;
    dao->fetchBalance = mysql_fetchBalance;
    dao->updateBalance = mysql_updateBalance;
    dao->getDailyWithdrawals = mysql_getDailyWithdrawals;
    dao->logWithdrawal = mysql_logWithdrawal;
    dao->logTransaction = mysql_logTransaction;
    dao->getMiniStatement = mysql_getMiniStatement;
    dao->getConnection = mysql_getConnection;
    dao->releaseConnection = mysql_releaseConnection;
    
    return dao;
}

// Validate card with PIN
static bool mysql_validateCard(int cardNumber, int pin) {
    char pinStr[20];
    sprintf(pinStr, "%d", pin);
    
    // In a real system, we would hash the PIN here before comparing
    // For this simplified implementation, we'll compare directly
    
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }
    
    char query[300];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s WHERE %s = %d AND pin_hash = '%s' AND %s = 'ACTIVE'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber, pinStr, COL_STATUS);
    
    bool valid = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                valid = atoi(row[0]) > 0;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return valid;
}

static float mysql_fetchBalance(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return -1.0f;
    }
    
    char query[400];
    sprintf(query, 
        "SELECT a.balance FROM %s a "
        "JOIN %s c ON a.%s = c.account_id "
        "WHERE c.%s = %d AND c.status = 'ACTIVE'", 
        TABLE_ACCOUNTS, 
        TABLE_CARDS, 
        COL_ACCOUNT_NUMBER, 
        COL_CARD_NUMBER, 
        cardNumber);
    
    float balance = -1.0f;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                balance = atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in fetchBalance: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return balance;
}

static bool mysql_updateBalance(int cardNumber, float newBalance) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    // First find the account ID for this card
    char find_account_query[400];
    sprintf(find_account_query, 
        "SELECT account_id FROM %s WHERE %s = %d", 
        TABLE_CARDS, 
        COL_CARD_NUMBER, 
        cardNumber);
    
    bool success = false;
    char account_id[50] = {0};
    
    if (mysql_query(conn, find_account_query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(account_id, row[0], sizeof(account_id) - 1);
                mysql_free_result(result);
                
                // Now update the balance
                char update_query[400];
                sprintf(update_query, 
                    "UPDATE %s SET balance = %.2f WHERE %s = '%s'", 
                    TABLE_ACCOUNTS, 
                    newBalance, 
                    COL_ACCOUNT_NUMBER, 
                    account_id);
                
                if (mysql_query(conn, update_query) == 0) {
                    // Log the balance update in audit log
                    char audit_query[500];
                    sprintf(audit_query,
                        "INSERT INTO %s (user, action, description, timestamp) "
                        "VALUES ('SYSTEM', 'BALANCE_UPDATE', 'Updated balance for account %s to %.2f', NOW())",
                        TABLE_AUDIT_LOG,
                        account_id,
                        newBalance);
                    
                    if (mysql_query(conn, audit_query) != 0) {
                        writeWarningLog("Failed to log balance update in audit log: %s", mysql_error(conn));
                    }
                    
                    success = true;
                    writeInfoLog("Balance updated for card %d, new balance: %.2f", cardNumber, newBalance);
                } else {
                    writeErrorLog("Failed to update balance: %s", mysql_error(conn));
                }
            } else {
                writeErrorLog("No account found for card %d", cardNumber);
                mysql_free_result(result);
            }
        }
    } else {
        writeErrorLog("MySQL query error finding account for card: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static bool mysql_blockCard(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE %s SET status = 'BLOCKED' WHERE %s = %d", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            // Update daily limits to zero for additional security
            char limit_query[300];
            sprintf(limit_query, 
                "UPDATE %s SET daily_atm_limit = 0.0, daily_pos_limit = 0.0, "
                "daily_online_limit = 0.0 WHERE %s = %d",
                TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
                
            if (mysql_query(conn, limit_query) != 0) {
                writeWarningLog("Failed to update card limits: %s", mysql_error(conn));
            }
            
            // Log the action in audit log
            char audit_query[300];
            sprintf(audit_query,
                "INSERT INTO %s (user, action, description, timestamp) "
                "VALUES ('SYSTEM', 'CARD_BLOCK', 'Blocked card %d', NOW())",
                TABLE_AUDIT_LOG, cardNumber);
                
            if (mysql_query(conn, audit_query) != 0) {
                writeWarningLog("Failed to log card block in audit log: %s", mysql_error(conn));
            }
            
            success = true;
            writeInfoLog("Card %d has been blocked", cardNumber);
        } else {
            writeErrorLog("Card %d not found for blocking", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in blockCard: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static bool mysql_unblockCard(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE %s SET status = 'ACTIVE' WHERE %s = %d", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            // Restore default limits
            char limit_query[300];
            sprintf(limit_query, 
                "UPDATE %s SET daily_atm_limit = 10000.00, daily_pos_limit = 50000.00, "
                "daily_online_limit = 30000.00 WHERE %s = %d",
                TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
                
            if (mysql_query(conn, limit_query) != 0) {
                writeWarningLog("Failed to restore card limits: %s", mysql_error(conn));
            }
            
            // Log the action in audit log
            char audit_query[300];
            sprintf(audit_query,
                "INSERT INTO %s (user, action, description, timestamp) "
                "VALUES ('SYSTEM', 'CARD_UNBLOCK', 'Unblocked card %d', NOW())",
                TABLE_AUDIT_LOG, cardNumber);
                
            if (mysql_query(conn, audit_query) != 0) {
                writeWarningLog("Failed to log card unblock in audit log: %s", mysql_error(conn));
            }
            
            success = true;
            writeInfoLog("Card %d has been unblocked", cardNumber);
        } else {
            writeErrorLog("Card %d not found for unblocking", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in unblockCard: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static bool mysql_updateCardPIN(int cardNumber, const char* newPINHash) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, "UPDATE %s SET pin_hash = '%s' WHERE %s = %d", 
            TABLE_CARDS, newPINHash, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            // Log the PIN change in audit log
            char audit_query[300];
            sprintf(audit_query,
                "INSERT INTO %s (user, action, description, timestamp) "
                "VALUES ('SYSTEM', 'PIN_CHANGE', 'Changed PIN for card %d', NOW())",
                TABLE_AUDIT_LOG, cardNumber);
                
            if (mysql_query(conn, audit_query) != 0) {
                writeWarningLog("Failed to log PIN change in audit log: %s", mysql_error(conn));
            }
            
            success = true;
            writeInfoLog("PIN hash updated for card %d", cardNumber);
        } else {
            writeErrorLog("Card %d not found for PIN update", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in updateCardPIN: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static bool mysql_getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    if (name == NULL || nameSize <= 0) {
        return false;
    }
    
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT c.name FROM %s c "
        "JOIN %s a ON c.%s = a.%s "
        "JOIN %s cd ON a.%s = cd.account_id "
        "WHERE cd.%s = %d", 
        TABLE_CUSTOMERS,
        TABLE_ACCOUNTS,
        COL_CUSTOMER_ID,
        COL_CUSTOMER_ID,
        TABLE_CARDS,
        COL_ACCOUNT_NUMBER,
        COL_CARD_NUMBER,
        cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(name, row[0], nameSize - 1);
                name[nameSize - 1] = '\0';
                success = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getCardHolderName: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static bool mysql_getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    if (phone == NULL || phoneSize <= 0) {
        return false;
    }
    
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT c.phone FROM %s c "
        "JOIN %s a ON c.%s = a.%s "
        "JOIN %s cd ON a.%s = cd.account_id "
        "WHERE cd.%s = %d", 
        TABLE_CUSTOMERS,
        TABLE_ACCOUNTS,
        COL_CUSTOMER_ID,
        COL_CUSTOMER_ID,
        TABLE_CARDS,
        COL_ACCOUNT_NUMBER,
        COL_CARD_NUMBER,
        cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(phone, row[0], phoneSize - 1);
                phone[phoneSize - 1] = '\0';
                success = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getCardHolderPhone: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}

static float mysql_getDailyWithdrawals(int cardNumber) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return 0.0f;
    }

    char date[11];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
    
    char query[400];
    sprintf(query, 
        "SELECT SUM(amount) FROM %s "
        "WHERE %s = %d AND DATE(withdrawal_date) = '%s'", 
        TABLE_DAILY_WITHDRAWALS, 
        COL_CARD_NUMBER, 
        cardNumber, 
        date);
    
    float total = 0.0f;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                total = atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getDailyWithdrawals: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return total;
}

static void mysql_logWithdrawal(int cardNumber, float amount) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection for logging withdrawal");
        return;
    }

    char date[20];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char query[400];
    sprintf(query, 
        "INSERT INTO %s (%s, amount, withdrawal_date) VALUES (%d, %.2f, '%s')",
        TABLE_DAILY_WITHDRAWALS, 
        COL_CARD_NUMBER, 
        cardNumber, 
        amount, 
        date);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("MySQL query error logging withdrawal: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
}

static bool mysql_logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection for logging transaction");
        return false;
    }

    char date[20];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm_info);
    
    char query[600];
    sprintf(query, 
        "INSERT INTO %s (%s, transaction_type, amount, status, transaction_date) "
        "VALUES (%d, '%s', %.2f, '%s', '%s')",
        TABLE_TRANSACTIONS, 
        COL_CARD_NUMBER,
        cardNumber, 
        transactionType, 
        amount, 
        success ? "Success" : "Failed", 
        date);
    
    bool result = true;
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("MySQL query error logging transaction: %s", mysql_error(conn));
        result = false;
    }
    
    mysql_releaseConnection(conn);
    return result;
}

static bool mysql_getMiniStatement(int cardNumber, Transaction* transactions, int maxTransactions, int* count) {
    if (transactions == NULL || count == NULL || maxTransactions <= 0) {
        return false;
    }
    
    MYSQL* conn = mysql_getConnection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT transaction_type, amount, status, transaction_date "
        "FROM %s "
        "WHERE %s = %d "
        "ORDER BY transaction_date DESC "
        "LIMIT %d", 
        TABLE_TRANSACTIONS, 
        COL_CARD_NUMBER, 
        cardNumber, 
        maxTransactions);
    
    bool success = false;
    *count = 0;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) && (*count < maxTransactions)) {
                if (row[0] && row[1] && row[2] && row[3]) {
                    strncpy(transactions[*count].type, row[0], sizeof(transactions[*count].type) - 1);
                    transactions[*count].amount = atof(row[1]);
                    strncpy(transactions[*count].status, row[2], sizeof(transactions[*count].status) - 1);
                    strncpy(transactions[*count].timestamp, row[3], sizeof(transactions[*count].timestamp) - 1);
                    (*count)++;
                }
            }
            mysql_free_result(result);
            success = true;
        }
    } else {
        writeErrorLog("MySQL query error in getMiniStatement: %s", mysql_error(conn));
    }
    
    mysql_releaseConnection(conn);
    return success;
}
