#include "../../../include/common/database/card_account_management.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Helper function to convert card number int to string
static void card_number_to_string(int cardNumber, char* cardStr, size_t bufferSize) {
    snprintf(cardStr, bufferSize, "%d", cardNumber);
}

/**
 * Block a card by updating its status in the core banking system
 * Time complexity: O(1) - single SQL operation
 */
bool cbs_block_card(int cardNumber) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_block_card");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE cbs_cards SET status = 'BLOCKED' WHERE card_number = '%s'", card_number_str);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[400];
            sprintf(audit_query, 
                "INSERT INTO cbs_audit_logs (action, entity_type, entity_id, details, status) "
                "VALUES ('CARD_BLOCK', 'CARD', '%s', 'Card blocked by system', 'SUCCESS')",
                card_number_str);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card block in audit log: %s", mysql_error(conn));
            }
            
            // Also set the limits to zero
            char limit_query[300];
            sprintf(limit_query, 
                "UPDATE cbs_cards SET daily_atm_limit = 0.0, daily_pos_limit = 0.0, "
                "daily_online_limit = 0.0 WHERE card_number = '%s'",
                card_number_str);
            
            if (mysql_query(conn, limit_query) != 0) {
                writeErrorLog("Failed to update card limits: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %s has been blocked", card_number_str);
        } else {
            writeErrorLog("Card %s not found for blocking", card_number_str);
        }
    } else {
        writeErrorLog("Failed to block card %s: %s", card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Unblock a card by updating its status in the core banking system
 * Time complexity: O(1) - single SQL operation
 */
bool cbs_unblock_card(int cardNumber) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_unblock_card");
        return false;
    }

    char query[200];
    sprintf(query, "UPDATE cbs_cards SET status = 'ACTIVE' WHERE card_number = '%s'", card_number_str);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[400];
            sprintf(audit_query, 
                "INSERT INTO cbs_audit_logs (action, entity_type, entity_id, details, status) "
                "VALUES ('CARD_UNBLOCK', 'CARD', '%s', 'Card unblocked by system', 'SUCCESS')",
                card_number_str);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log card unblock in audit log: %s", mysql_error(conn));
            }
            
            // Restore default limits
            char limit_query[300];
            sprintf(limit_query, 
                "UPDATE cbs_cards SET daily_atm_limit = 10000.00, daily_pos_limit = 50000.00, "
                "daily_online_limit = 30000.00 WHERE card_number = '%s'",
                card_number_str);
            
            if (mysql_query(conn, limit_query) != 0) {
                writeErrorLog("Failed to restore card limits: %s", mysql_error(conn));
            }
            
            writeInfoLog("Card %s has been unblocked", card_number_str);
        } else {
            writeErrorLog("Card %s not found for unblocking", card_number_str);
        }
    } else {
        writeErrorLog("Failed to unblock card %s: %s", card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Get daily withdrawals for a card from the core banking system
 * Time complexity: O(1) - indexed query with date filter
 */
float cbs_get_daily_withdrawals(int cardNumber) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_daily_withdrawals");
        return 0.0f;
    }

    char query[300];
    sprintf(query, 
            "SELECT SUM(amount) FROM cbs_daily_withdrawals "
            "WHERE card_number = '%s' AND withdrawal_date = CURDATE() "
            "AND status = 'COMPLETED'", 
            card_number_str);
    
    float total = 0.0f;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                total = (float)atof(row[0]);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to query daily withdrawals for card %s: %s", 
                     card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return total;
}

/**
 * Verify if a card is valid using the core banking system
 * Time complexity: O(1) - indexed query by card number
 */
bool cbs_validate_card(int cardNumber, int pin) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    char pin_str[20];
    snprintf(pin_str, sizeof(pin_str), "%d", pin);
    
    // In a real system, we would hash the PIN here
    // For demonstration purposes, we're using plain text
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_validate_card");
        return false;
    }

    // Note: In production, you would compare the hash, not the raw PIN
    // This is just for demonstration purposes
    char query[300];
    sprintf(query, 
            "SELECT COUNT(*) FROM cbs_cards "
            "WHERE card_number = '%s' AND pin_hash = '%s' AND status = 'ACTIVE'", 
            card_number_str, pin_str);
    
    bool valid = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && atoi(row[0]) > 0) {
                valid = true;
                writeInfoLog("Card %s validated successfully", card_number_str);
            } else {
                writeInfoLog("Card %s validation failed", card_number_str);
                
                // Log failed attempt
                char audit_query[400];
                sprintf(audit_query, 
                    "INSERT INTO cbs_audit_logs (action, entity_type, entity_id, details, status) "
                    "VALUES ('PIN_VALIDATION', 'CARD', '%s', 'Failed PIN validation attempt', 'FAILURE')",
                    card_number_str);
                
                mysql_query(conn, audit_query);
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to validate card %s: %s", card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return valid;
}

/**
 * Update card PIN in the core banking system
 * Time complexity: O(1) - indexed update by card number
 */
bool cbs_update_pin(int cardNumber, int newPin) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    char new_pin_str[20];
    snprintf(new_pin_str, sizeof(new_pin_str), "%d", newPin);
    
    // In a real system, we would hash the PIN here
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_update_pin");
        return false;
    }

    // Note: In production, you would store the hash, not the raw PIN
    char query[300];
    sprintf(query, 
            "UPDATE cbs_cards SET pin_hash = '%s' "
            "WHERE card_number = '%s' AND status = 'ACTIVE'", 
            new_pin_str, card_number_str);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log the PIN change
            char transaction_id[37];
            char account_number[25];
            
            // Get account number for this card
            char acct_query[200];
            sprintf(acct_query, "SELECT account_id FROM cbs_cards WHERE card_number = '%s'", 
                    card_number_str);
            
            if (mysql_query(conn, acct_query) == 0) {
                MYSQL_RES *result = mysql_store_result(conn);
                if (result) {
                    MYSQL_ROW row = mysql_fetch_row(result);
                    if (row && row[0]) {
                        strncpy(account_number, row[0], sizeof(account_number) - 1);
                        account_number[sizeof(account_number) - 1] = '\0';
                        
                        // Log transaction
                        char trans_query[500];
                        sprintf(trans_query, 
                            "INSERT INTO cbs_transactions "
                            "(transaction_id, card_number, account_number, transaction_type, "
                            "channel, amount, balance_before, balance_after, value_date, status) "
                            "VALUES (UUID(), '%s', '%s', 'PIN_CHANGE', 'ATM', 0.00, 0.00, 0.00, "
                            "CURDATE(), 'SUCCESS')",
                            card_number_str, account_number);
                        
                        if (mysql_query(conn, trans_query) != 0) {
                            writeErrorLog("Failed to log PIN change transaction: %s", 
                                         mysql_error(conn));
                        }
                    }
                    mysql_free_result(result);
                }
            }
            
            writeInfoLog("PIN updated for card %s", card_number_str);
        } else {
            writeErrorLog("Card %s not found for PIN update or already has this PIN", 
                         card_number_str);
        }
    } else {
        writeErrorLog("Failed to update PIN for card %s: %s", card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * Check if a card exists in the core banking system
 * Time complexity: O(1) - indexed query by card number
 */
bool cbs_card_exists(int cardNumber) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_card_exists");
        return false;
    }

    char query[200];
    sprintf(query, "SELECT COUNT(*) FROM cbs_cards WHERE card_number = '%s'", 
            card_number_str);
    
    bool exists = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && atoi(row[0]) > 0) {
                exists = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to check if card %s exists: %s", 
                     card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return exists;
}

/**
 * Check if a card is active in the core banking system
 * Time complexity: O(1) - indexed query by card number
 */
bool cbs_is_card_active(int cardNumber) {
    char card_number_str[20];
    card_number_to_string(cardNumber, card_number_str, sizeof(card_number_str));
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_is_card_active");
        return false;
    }

    char query[200];
    sprintf(query, "SELECT status FROM cbs_cards WHERE card_number = '%s'", 
            card_number_str);
    
    bool isActive = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0] && strcmp(row[0], "ACTIVE") == 0) {
                isActive = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to check if card %s is active: %s", 
                     card_number_str, mysql_error(conn));
    }
    
    db_release_connection(conn);
    return isActive;
}

/* Legacy function names for backward compatibility */

bool blockCard(int cardNumber) {
    return cbs_block_card(cardNumber);
}

bool unblockCard(int cardNumber) {
    return cbs_unblock_card(cardNumber);
}

float getDailyWithdrawals(int cardNumber) {
    return cbs_get_daily_withdrawals(cardNumber);
}