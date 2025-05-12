#include "../../include/common/database/database.h"
#include "../../include/common/database/db_config.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/paths.h"
#include "../../include/common/security/hash_utils.h"
#include "../../include/common/utils/encryption_utils.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

// Helper function to get current date as a string (YYYY-MM-DD)
static void getCurrentDate(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d", t);
}

// Helper function to get current timestamp as a string (YYYY-MM-DD HH:MM:SS)
static void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Initialize database
bool initialize_database(void) {
    // Initialize database connection pool
    if (!db_init()) {
        writeErrorLog("Failed to initialize database connection pool");
        return false;
    }

    writeInfoLog("Database connection initialized successfully");
    return true;
}

// Check if a card number exists in the database
bool doesCardExist(int cardNumber) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
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
                exists = atoi(row[0]) > 0;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in doesCardExist: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return exists;
}

// Check if a card is active
bool isCardActive(int cardNumber) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
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

// Validate card with PIN (legacy method, for backward compatibility)
bool validateCard(int cardNumber, int pin) {
    char pinStr[20];
    sprintf(pinStr, "%d", pin);
    char* pinHash = sha256_hash(pinStr);
    bool result = validateCardWithHash(cardNumber, pinHash);
    free(pinHash);
    return result;
}

// Validate card with PIN hash
bool validateCardWithHash(int cardNumber, const char* pinHash) {
    if (pinHash == NULL) {
        writeErrorLog("NULL PIN hash provided to validateCardWithHash");
        return false;
    }
    
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[300];
    sprintf(query, "SELECT pin_hash FROM %s WHERE %s = '%d' AND status = 'ACTIVE'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool valid = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                // Use secure hash comparison
                valid = secure_hash_compare(row[0], pinHash);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in validateCardWithHash: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return valid;
}

// Update PIN for a card (legacy method)
bool updatePIN(int cardNumber, int newPin) {
    char pinStr[20];
    sprintf(pinStr, "%d", newPin);
    char* pinHash = sha256_hash(pinStr);
    bool result = updatePINHash(cardNumber, pinHash);
    free(pinHash);
    return result;
}

// Update PIN hash for a card
bool updatePINHash(int cardNumber, const char* pinHash) {
    if (pinHash == NULL) {
        writeErrorLog("NULL PIN hash provided to updatePINHash");
        return false;
    }
    
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[300];
    sprintf(query, "UPDATE %s SET pin_hash = '%s' WHERE %s = '%d'", 
            TABLE_CARDS, pinHash, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log the PIN change in audit log
            char audit_query[500];
            sprintf(audit_query, 
                "INSERT INTO %s (action, entity_type, entity_id, details) VALUES ('PIN_CHANGE', 'CARD', '%d', 'PIN updated via ATM')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log PIN change in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("PIN hash updated for card %d", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in updatePINHash: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Get card holder's name by looking up customer info by card number
bool getCardHolderName(int cardNumber, char* name, size_t nameSize) {
    if (name == NULL || nameSize <= 0) {
        return false;
    }
    
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT c.name FROM %s c "
        "JOIN %s a ON c.%s = a.%s "
        "JOIN %s cd ON a.%s = cd.account_id "
        "WHERE cd.%s = '%d'", 
        TABLE_CUSTOMERS, 
        TABLE_ACCOUNTS, COL_CUSTOMER_ID, COL_CUSTOMER_ID,
        TABLE_CARDS, COL_ACCOUNT_NUMBER, 
        COL_CARD_NUMBER, cardNumber);
    
    bool found = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(name, row[0], nameSize - 1);
                name[nameSize - 1] = '\0';
                found = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getCardHolderName: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return found;
}

// Get card holder phone
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize) {
    if (phone == NULL || phoneSize <= 0) {
        return false;
    }
    
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT c.phone FROM %s c "
        "JOIN %s a ON c.%s = a.%s "
        "JOIN %s cd ON a.%s = cd.account_id "
        "WHERE cd.%s = '%d'", 
        TABLE_CUSTOMERS, 
        TABLE_ACCOUNTS, COL_CUSTOMER_ID, COL_CUSTOMER_ID,
        TABLE_CARDS, COL_ACCOUNT_NUMBER, 
        COL_CARD_NUMBER, cardNumber);
    
    bool found = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                strncpy(phone, row[0], phoneSize - 1);
                phone[phoneSize - 1] = '\0';
                found = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getCardHolderPhone: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return found;
}

// Fetch account balance
bool fetchBalance(int cardNumber, float* balance) {
    if (balance == NULL) {
        writeErrorLog("NULL balance pointer provided to fetchBalance");
        return false;
    }
    
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[400];
    sprintf(query, 
        "SELECT a.balance FROM %s a "
        "JOIN %s c ON a.%s = c.account_id "
        "WHERE c.%s = '%d' AND c.status = 'ACTIVE'", 
        TABLE_ACCOUNTS, 
        TABLE_CARDS, COL_ACCOUNT_NUMBER, 
        COL_CARD_NUMBER, cardNumber);
    
    bool found = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                *balance = atof(row[0]);
                found = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in fetchBalance: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return found;
}

// Update account balance
bool updateBalance(int cardNumber, float newBalance) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    // Find the associated account for this card
    char find_account_query[400];
    sprintf(find_account_query, 
        "SELECT account_id FROM %s WHERE %s = '%d'", 
        TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, find_account_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                char account_id[21];
                strncpy(account_id, row[0], sizeof(account_id) - 1);
                account_id[sizeof(account_id) - 1] = '\0';
                
                // Update the balance
                char update_query[400];
                sprintf(update_query, 
                    "UPDATE %s SET balance = %.2f, last_transaction = NOW() WHERE %s = '%s'", 
                    TABLE_ACCOUNTS, newBalance, COL_ACCOUNT_NUMBER, account_id);
                
                if (mysql_query(conn, update_query) == 0) {
                    if (mysql_affected_rows(conn) > 0) {
                        success = true;
                        writeInfoLog("Balance updated for account %s to %.2f", account_id, newBalance);
                    }
                } else {
                    writeErrorLog("MySQL query error updating balance: %s", mysql_error(conn));
                }
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error finding account for card: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Log withdrawal for tracking and daily limits
void logWithdrawal(int cardNumber, float amount) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection for logging withdrawal");
        return;
    }

    // Get account number for the card
    char query[400];
    char date[20];
    getCurrentDate(date, sizeof(date));
    
    sprintf(query, 
        "SELECT account_id FROM %s WHERE %s = '%d'", 
        TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                char account_id[21];
                strncpy(account_id, row[0], sizeof(account_id) - 1);
                account_id[sizeof(account_id) - 1] = '\0';
                
                // Get the current balance for before/after values
                float current_balance = 0.0;
                char balance_query[200];
                sprintf(balance_query, 
                    "SELECT balance FROM %s WHERE %s = '%s'", 
                    TABLE_ACCOUNTS, COL_ACCOUNT_NUMBER, account_id);
                
                if (mysql_query(conn, balance_query) == 0) {
                    MYSQL_RES *balance_result = mysql_store_result(conn);
                    if (balance_result) {
                        MYSQL_ROW balance_row = mysql_fetch_row(balance_result);
                        if (balance_row && balance_row[0]) {
                            current_balance = atof(balance_row[0]);
                        }
                        mysql_free_result(balance_result);
                    }
                }
                
                // Insert withdrawal into transactions
                char transaction_id[37]; // UUID format
                sprintf(transaction_id, "W-%08x-%04x-%04x-%04x-%012x",
                    (unsigned int)time(NULL),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffffffffffff));
                
                char transaction_query[600];
                sprintf(transaction_query, 
                    "INSERT INTO %s (%s, %s, account_number, transaction_type, "
                    "amount, balance_before, balance_after, status, remarks) "
                    "VALUES ('%s', '%d', '%s', 'WITHDRAWAL', %.2f, %.2f, %.2f, 'SUCCESS', 'ATM Withdrawal')",
                    TABLE_TRANSACTIONS, 
                    COL_TRANSACTION_ID, COL_CARD_NUMBER, 
                    transaction_id, cardNumber, account_id, 
                    amount, current_balance, current_balance - amount);
                
                if (mysql_query(conn, transaction_query) != 0) {
                    writeErrorLog("MySQL query error logging transaction: %s", mysql_error(conn));
                }
                
                // Also track in daily withdrawals table for limit enforcement
                char daily_query[400];
                sprintf(daily_query, 
                    "INSERT INTO %s (%s, amount, withdrawal_date) VALUES ('%d', %.2f, '%s')",
                    TABLE_DAILY_WITHDRAWALS, COL_CARD_NUMBER, cardNumber, amount, date);
                
                if (mysql_query(conn, daily_query) != 0) {
                    writeErrorLog("MySQL query error logging daily withdrawal: %s", mysql_error(conn));
                }
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error getting account for withdrawal: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
}

// Function to generate a card number in the format found in init_data_files.c
char* generateCardNumber() {
    char* cardNumber = (char*)malloc(20); // 16 digits + 3 dashes + null terminator
    if (!cardNumber) {
        writeErrorLog("Failed to allocate memory for card number");
        return NULL;
    }
    
    // Format: XXXX-XXXX-XXXX-XXXX
    sprintf(cardNumber, "%04d-%04d-%04d-%04d", 
            4000 + (rand() % 999),  // Start with 4 for "Visa-like" numbers
            1000 + (rand() % 9000),
            1000 + (rand() % 9000),
            1000 + (rand() % 9000));
    
    return cardNumber;
}

// Function to generate a 3-digit CVV
int generateCVV() {
    return 100 + (rand() % 900); // 3-digit number between 100 and 999
}

// Block a card
bool blockCard(int cardNumber) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE %s SET status = 'BLOCKED' WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[400];
            sprintf(audit_query, 
                "INSERT INTO %s (action, entity_type, entity_id, details) "
                "VALUES ('CARD_BLOCK', 'CARD', '%d', 'Card blocked via system')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card block in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %d has been blocked", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in blockCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Unblock a card
bool unblockCard(int cardNumber) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE %s SET status = 'ACTIVE' WHERE %s = '%d'", 
            TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[400];
            sprintf(audit_query, 
                "INSERT INTO %s (action, entity_type, entity_id, details) "
                "VALUES ('CARD_UNBLOCK', 'CARD', '%d', 'Card unblocked via system')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card unblock in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %d has been unblocked", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in unblockCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Log transaction
bool logTransaction(int cardNumber, const char* transactionType, float amount, bool success) {
    MYSQL *conn = (MYSQL *)db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection for logging transaction");
        return false;
    }

    // Get account number for the card
    char query[500];
    sprintf(query, 
        "SELECT account_id FROM %s WHERE %s = '%d'", 
        TABLE_CARDS, COL_CARD_NUMBER, cardNumber);
    
    bool result = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *queryResult = mysql_store_result(conn);
        if (queryResult) {
            MYSQL_ROW row = mysql_fetch_row(queryResult);
            if (row && row[0]) {
                char account_id[21];
                strncpy(account_id, row[0], sizeof(account_id) - 1);
                account_id[sizeof(account_id) - 1] = '\0';
                
                // Get current balance
                float current_balance = 0.0;
                char balance_query[200];
                sprintf(balance_query, 
                    "SELECT balance FROM %s WHERE %s = '%s'", 
                    TABLE_ACCOUNTS, COL_ACCOUNT_NUMBER, account_id);
                
                if (mysql_query(conn, balance_query) == 0) {
                    MYSQL_RES *balance_result = mysql_store_result(conn);
                    if (balance_result) {
                        MYSQL_ROW balance_row = mysql_fetch_row(balance_result);
                        if (balance_row && balance_row[0]) {
                            current_balance = atof(balance_row[0]);
                        }
                        mysql_free_result(balance_result);
                    }
                }
                
                // Calculate balance after, based on transaction type
                float balance_after = current_balance;
                if (strcmp(transactionType, "WITHDRAWAL") == 0) {
                    balance_after = current_balance - amount;
                } else if (strcmp(transactionType, "DEPOSIT") == 0) {
                    balance_after = current_balance + amount;
                }
                
                // Create a UUID-like transaction ID
                char transaction_id[37];
                sprintf(transaction_id, "T-%08x-%04x-%04x-%04x-%012x",
                    (unsigned int)time(NULL),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffff),
                    (unsigned int)(rand() & 0xffffffffffff));
                
                // Log the transaction
                char transaction_query[600];
                sprintf(transaction_query, 
                    "INSERT INTO %s (%s, %s, account_number, transaction_type, "
                    "amount, balance_before, balance_after, status, remarks) "
                    "VALUES ('%s', '%d', '%s', '%s', %.2f, %.2f, %.2f, '%s', 'ATM Transaction')",
                    TABLE_TRANSACTIONS, 
                    COL_TRANSACTION_ID, COL_CARD_NUMBER,
                    transaction_id, cardNumber, account_id, 
                    transactionType, amount, current_balance, balance_after,
                    success ? "SUCCESS" : "FAILED");
                
                if (mysql_query(conn, transaction_query) == 0) {
                    result = true;
                    
                    // Log to system logs
                    if (success) {
                        writeInfoLog("Transaction %s: %s for card %d, amount %.2f", 
                                    transaction_id, transactionType, cardNumber, amount);
                    } else {
                        writeWarningLog("Failed transaction %s: %s for card %d, amount %.2f", 
                                       transaction_id, transactionType, cardNumber, amount);
                    }
                } else {
                    writeErrorLog("MySQL query error logging transaction: %s", mysql_error(conn));
                }
            }
            mysql_free_result(queryResult);
        }
    } else {
        writeErrorLog("MySQL query error getting account for transaction: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return result;
}

