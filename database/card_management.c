#include "../../include/common/database/card_account_management.h"
#include "../../include/common/database/db_config.h"
#include "../../include/common/utils/logger.h"
#include "../../include/common/paths.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Forward declarations from database.c
extern bool doesCardExist(int cardNumber);
extern bool isCardActive(int cardNumber);
extern MYSQL* db_get_connection();
extern void db_release_connection(MYSQL* conn);

// Block a card by updating its status to "BLOCKED"
bool blockCard(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in blockCard");
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
                "VALUES ('CARD_BLOCK', 'CARD', '%d', 'Card blocked by administrator')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card block in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %d has been blocked", cardNumber);
        } else {
            writeErrorLog("Card %d not found for blocking", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in blockCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Unblock a card by updating its status to "ACTIVE"
bool unblockCard(int cardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in unblockCard");
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
                "VALUES ('CARD_UNBLOCK', 'CARD', '%d', 'Card unblocked by administrator')",
                TABLE_AUDIT_LOGS, cardNumber);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card unblock in audit log: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %d has been unblocked", cardNumber);
        } else {
            writeErrorLog("Card %d not found for unblocking", cardNumber);
        }
    } else {
        writeErrorLog("MySQL query error in unblockCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Issue a new card for a customer
bool issueCard(const char* accountNumber, const char* customerName, int* newCardNumber) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in issueCard");
        return false;
    }
    
    // Generate card details
    int cardNum = 4000000000000000 + (rand() % 999999999);
    int cvv = 100 + (rand() % 900);
    char card_id[20];
    sprintf(card_id, "C%010d", (rand() % 9999999999) + 1);
    
    // Generate expiry date (current month + 5 years)
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    t->tm_year += 5;  // Add 5 years
    char expiry_date[11];
    strftime(expiry_date, sizeof(expiry_date), "%Y-%m-%d", t);
    
    // Generate default PIN hash (1234)
    const char* default_pin_hash = "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4"; // SHA-256 hash of "1234"
    
    // Insert the new card into the database
    char query[600];
    sprintf(query, 
        "INSERT INTO %s (%s, account_id, %s, card_type, expiry_date, cvv, pin_hash, status, issue_date, daily_limit) "
        "VALUES ('%s', '%s', '%d', 'DEBIT', '%s', '%d', '%s', 'ACTIVE', NOW(), 10000.00)",
        TABLE_CARDS, COL_CARD_ID, COL_CARD_NUMBER,
        card_id, accountNumber, cardNum, expiry_date, cvv, default_pin_hash);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        *newCardNumber = cardNum;
        success = true;
        
        // Log to audit log
        char audit_query[400];
        sprintf(audit_query, 
            "INSERT INTO %s (action, entity_type, entity_id, details) "
            "VALUES ('CARD_ISSUE', 'CARD', '%d', 'New card issued for account %s')",
            TABLE_AUDIT_LOGS, cardNum, accountNumber);
        
        if (mysql_query(conn, audit_query) != 0) {
            writeErrorLog("Failed to log card issue in audit log: %s", mysql_error(conn));
        }
        
        writeInfoLog("New card %d issued for account %s", cardNum, accountNumber);
    } else {
        writeErrorLog("MySQL query error in issueCard: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

// Get card details
bool getCardDetails(int cardNumber, CardDetails* details) {
    if (!details) {
        writeErrorLog("NULL details pointer provided to getCardDetails");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in getCardDetails");
        return false;
    }

    char query[500];
    sprintf(query, 
        "SELECT c.%s, c.account_id, c.card_type, c.expiry_date, c.status, c.daily_limit, "
        "a.%s, a.account_type, cu.name as customer_name "
        "FROM %s c "
        "JOIN %s a ON c.account_id = a.%s "
        "JOIN %s cu ON a.%s = cu.%s "
        "WHERE c.%s = '%d'",
        COL_CARD_ID, COL_ACCOUNT_NUMBER,
        TABLE_CARDS,
        TABLE_ACCOUNTS, COL_ACCOUNT_NUMBER,
        TABLE_CUSTOMERS, COL_CUSTOMER_ID, COL_CUSTOMER_ID,
        COL_CARD_NUMBER, cardNumber);
    
    bool found = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                // Copy data to details structure
                strncpy(details->cardID, row[0], sizeof(details->cardID) - 1);
                details->cardID[sizeof(details->cardID) - 1] = '\0';
                
                strncpy(details->accountID, row[1], sizeof(details->accountID) - 1);
                details->accountID[sizeof(details->accountID) - 1] = '\0';
                
                strncpy(details->cardType, row[2], sizeof(details->cardType) - 1);
                details->cardType[sizeof(details->cardType) - 1] = '\0';
                
                strncpy(details->expiryDate, row[3], sizeof(details->expiryDate) - 1);
                details->expiryDate[sizeof(details->expiryDate) - 1] = '\0';
                
                strncpy(details->status, row[4], sizeof(details->status) - 1);
                details->status[sizeof(details->status) - 1] = '\0';
                
                details->dailyLimit = atof(row[5]);
                details->cardNumber = cardNumber;
                
                strncpy(details->accountNumber, row[6], sizeof(details->accountNumber) - 1);
                details->accountNumber[sizeof(details->accountNumber) - 1] = '\0';
                
                strncpy(details->accountType, row[7], sizeof(details->accountType) - 1);
                details->accountType[sizeof(details->accountType) - 1] = '\0';
                
                strncpy(details->customerName, row[8], sizeof(details->customerName) - 1);
                details->customerName[sizeof(details->customerName) - 1] = '\0';
                
                found = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("MySQL query error in getCardDetails: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return found;
}