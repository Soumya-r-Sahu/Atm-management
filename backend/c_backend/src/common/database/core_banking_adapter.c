#include "../../../include/common/database/core_banking_adapter.h"
#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/database.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include "../../../include/atm/transaction/transaction_types.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper function to convert card number to account number
static bool get_account_number_from_card(int card_number, char* account_number_out, size_t buffer_size) {
    // Assume CBS stores mappings between cards and accounts
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in get_account_number_from_card");
        return false;
    }

    char query[200];
    sprintf(query, "SELECT account_number FROM cbs_card_account_mapping WHERE card_number = '%d'", card_number);

    if (mysql_query(conn, query)) {
        writeErrorLog(mysql_error(conn));
        db_release_connection(conn);
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog(mysql_error(conn));
        db_release_connection(conn);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    bool success = false;
    
    if (row) {
        strncpy(account_number_out, row[0], buffer_size - 1);
        account_number_out[buffer_size - 1] = '\0';
        success = true;
    } else {
        writeErrorLog("No account found for card number %d", card_number);
    }

    mysql_free_result(result);
    db_release_connection(conn);
    return success;
}

// Convert TransactionType to TRANSACTION_TYPE enum
static TRANSACTION_TYPE convert_transaction_type(TransactionType atm_type) {
    switch (atm_type) {
        case TRANSACTION_BALANCE:
            return TRANSACTION_BALANCE_INQUIRY;
        case TRANSACTION_WITHDRAWAL:
            return TRANSACTION_WITHDRAWAL;
        case TRANSACTION_DEPOSIT:
            return TRANSACTION_DEPOSIT;
        case TRANSACTION_TRANSFER:
            return TRANSACTION_TRANSFER;
        case TRANSACTION_PIN_CHANGE:
            return TRANSACTION_PIN_CHANGE;
        case TRANSACTION_MINI_STATEMENT:
            return TRANSACTION_MINI_STATEMENT;
        case TRANSACTION_BILL_PAYMENT:
            return TRANSACTION_PAYMENT;
        default:
            // Default to balance inquiry as a safe fallback
            return TRANSACTION_BALANCE_INQUIRY;
    }
}

// Adapter for fetchBalance function
bool cbs_adapter_fetch_balance(int card_number, float* balance) {
    char account_number[20];
    if (!get_account_number_from_card(card_number, account_number, sizeof(account_number))) {
        return false;
    }

    double cbs_balance = 0.0;
    bool result = cbs_get_account_balance(account_number, &cbs_balance);
    
    if (result) {
        *balance = (float)cbs_balance;
    }
    
    return result;
}

// Adapter for updateBalance function
bool cbs_adapter_update_balance(int card_number, float new_balance) {
    // This function is now obsolete with CBS - use cbs_process_transaction instead
    writeInfoLog("cbs_adapter_update_balance called, but this function is deprecated");
    return false;
}

// Adapter for transaction processing
bool cbs_adapter_process_transaction(int card_number, TransactionType type, 
                                    const char* channel, float amount, 
                                    char* transaction_id_out) {
    char account_number[20];
    if (!get_account_number_from_card(card_number, account_number, sizeof(account_number))) {
        return false;
    }

    TRANSACTION_TYPE cbs_type = convert_transaction_type(type);
    return cbs_process_transaction(account_number, cbs_type, channel, (double)amount, transaction_id_out);
}

// Adapter for fund transfers
bool cbs_adapter_transfer_funds(int source_card, int dest_card, float amount, char* transaction_id_out) {
    char source_account[20];
    char dest_account[20];
    
    if (!get_account_number_from_card(source_card, source_account, sizeof(source_account))) {
        return false;
    }
    
    if (!get_account_number_from_card(dest_card, dest_account, sizeof(dest_account))) {
        return false;
    }
    
    return cbs_transfer_funds(source_account, dest_account, (double)amount, "ATM_TRANSFER", transaction_id_out);
}

// Adapter for daily withdrawal limit check
bool cbs_adapter_check_withdrawal_limit(int card_number, float amount, double* remaining_limit) {
    char account_number[20];
    if (!get_account_number_from_card(card_number, account_number, sizeof(account_number))) {
        return false;
    }
    
    char card_number_str[16];
    sprintf(card_number_str, "%d", card_number);
    
    return cbs_check_withdrawal_limit(card_number_str, (double)amount, "ATM", remaining_limit);
}

// Adapter for mini statement
bool cbs_adapter_get_mini_statement(int card_number, Transaction* atm_transactions, int* count, int max_records) {
    char account_number[20];
    if (!get_account_number_from_card(card_number, account_number, sizeof(account_number))) {
        return false;
    }
    
    // Create temporary storage for CBS records
    TransactionRecord* cbs_records = (TransactionRecord*)malloc(max_records * sizeof(TransactionRecord));
    if (!cbs_records) {
        writeErrorLog("Memory allocation failed in cbs_adapter_get_mini_statement");
        return false;
    }
    
    bool result = cbs_get_mini_statement(account_number, cbs_records, count, max_records);
    
    if (result) {
        // Convert CBS records to ATM transaction format
        for (int i = 0; i < *count; i++) {
            atm_transactions[i].card_number = card_number;
            atm_transactions[i].id = i + 1; // Just use incremental ids
            
            // Convert transaction type string to enum
            if (strcmp(cbs_records[i].transaction_type, "WITHDRAWAL") == 0) {
                atm_transactions[i].type = TRANSACTION_WITHDRAWAL;
                strcpy(atm_transactions[i].transaction_type, "Withdrawal");
            } 
            else if (strcmp(cbs_records[i].transaction_type, "DEPOSIT") == 0) {
                atm_transactions[i].type = TRANSACTION_DEPOSIT;
                strcpy(atm_transactions[i].transaction_type, "Deposit");
            }
            else if (strcmp(cbs_records[i].transaction_type, "TRANSFER") == 0) {
                atm_transactions[i].type = TRANSACTION_TRANSFER;
                strcpy(atm_transactions[i].transaction_type, "Transfer");
            }
            else if (strcmp(cbs_records[i].transaction_type, "BALANCE_INQUIRY") == 0) {
                atm_transactions[i].type = TRANSACTION_BALANCE;
                strcpy(atm_transactions[i].transaction_type, "Balance");
            }
            else {
                // Default
                atm_transactions[i].type = TRANSACTION_BALANCE;
                strncpy(atm_transactions[i].transaction_type, cbs_records[i].transaction_type, 19);
                atm_transactions[i].transaction_type[19] = '\0';
            }
            
            atm_transactions[i].amount = (float)cbs_records[i].amount;
            strncpy(atm_transactions[i].timestamp, cbs_records[i].date, 19);
            atm_transactions[i].timestamp[19] = '\0';
            strncpy(atm_transactions[i].status, cbs_records[i].status, 9);
            atm_transactions[i].status[9] = '\0';
        }
    }
    
    free(cbs_records);
    return result;
}

// Adapter for card blocking
bool cbs_adapter_block_card(int card_number) {
    char card_number_str[16];
    sprintf(card_number_str, "%d", card_number);
    
    // Set limits to zero to effectively block the card
    return cbs_update_card_limits(card_number_str, 0.0, 0.0, 0.0);
}

// Adapter for card unblocking
bool cbs_adapter_unblock_card(int card_number) {
    char card_number_str[16];
    sprintf(card_number_str, "%d", card_number);
    
    // Restore default limits
    return cbs_update_card_limits(card_number_str, 25000.0, 50000.0, 100000.0);
}

/**
 * @brief Get detailed information about a card
 * @param card_number Card number to query
 * @param holder_name_out Output buffer for cardholder name
 * @param holder_name_size Size of holder_name_out buffer
 * @param account_number_out Output buffer for account number
 * @param account_number_size Size of account_number_out buffer
 * @param expiry_date_out Output buffer for expiry date
 * @param expiry_date_size Size of expiry_date_out buffer
 * @param is_active_out Output parameter for card active status
 * @param daily_limit_out Output parameter for daily transaction limit
 * @param card_type_out Output buffer for card type
 * @param card_type_size Size of card_type_out buffer
 * @return true if successful, false otherwise
 */
bool cbs_get_card_details(int card_number,
                         char* holder_name_out, size_t holder_name_size,
                         char* account_number_out, size_t account_number_size,
                         char* expiry_date_out, size_t expiry_date_size,
                         int* is_active_out,
                         double* daily_limit_out,
                         char* card_type_out, size_t card_type_size) {
    
    if (!holder_name_out || !account_number_out || !expiry_date_out || 
        !is_active_out || !daily_limit_out || !card_type_out) {
        writeErrorLog("Invalid parameters in cbs_get_card_details");
        return false;
    }
    
    // Get database connection
    MYSQL* conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_card_details");
        return false;
    }
    
    // First, get account number associated with the card
    if (!get_account_number_from_card(card_number, account_number_out, account_number_size)) {
        writeErrorLog("Failed to get account number for card %d", card_number);
        db_release_connection(conn);
        return false;
    }
    
    // Now, query card details
    char query[512];
    sprintf(query, 
        "SELECT c.card_type, c.expiry_date, c.is_active, c.daily_limit, a.holder_name "
        "FROM cbs_cards c "
        "JOIN cbs_accounts a ON c.account_number = a.account_number "
        "WHERE c.card_number = '%d'", 
        card_number);
    
    if (mysql_query(conn, query)) {
        writeErrorLog("MySQL query error in cbs_get_card_details: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result in cbs_get_card_details: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    if (mysql_num_rows(result) == 0) {
        writeErrorLog("No card details found for card number %d", card_number);
        mysql_free_result(result);
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    
    // Process the results
    if (row) {
        // Card type
        strncpy(card_type_out, row[0] ? row[0] : "Unknown", card_type_size - 1);
        card_type_out[card_type_size - 1] = '\0';
        
        // Expiry date
        strncpy(expiry_date_out, row[1] ? row[1] : "Unknown", expiry_date_size - 1);
        expiry_date_out[expiry_date_size - 1] = '\0';
        
        // Card status
        *is_active_out = (row[2] && strcmp(row[2], "1") == 0) ? 1 : 0;
        
        // Daily limit
        *daily_limit_out = row[3] ? atof(row[3]) : 0.0;
        
        // Holder name
        strncpy(holder_name_out, row[4] ? row[4] : "Unknown", holder_name_size - 1);
        holder_name_out[holder_name_size - 1] = '\0';
    }
    
    mysql_free_result(result);
    db_release_connection(conn);
    
    writeInfoLog("Successfully retrieved card details for card %d", card_number);
    return true;
}
