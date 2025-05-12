/**
 * @file bill_payment.c
 * @brief Implementation of bill payment functionality for the Core Banking System
 */

#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <uuid/uuid.h>

/**
 * Generate a unique transaction ID using UUID
 */
static void generate_transaction_id(char* transaction_id, size_t size) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, transaction_id);
}

/**
 * @brief Process bill payment through the Core Banking System
 * @param card_number Card number
 * @param bill_type Type of bill (ELECTRICITY, WATER, etc.)
 * @param bill_reference Reference number for the bill
 * @param amount Amount to pay
 * @param transaction_id_out Output parameter for transaction ID
 * @return true if payment was successful, false otherwise
 */
bool cbs_process_bill_payment(const char* card_number, 
                              const char* bill_type,
                              const char* bill_reference,
                              double amount, 
                              char* transaction_id_out) {
    if (!card_number || !bill_type || !bill_reference || amount <= 0 || !transaction_id_out) {
        writeErrorLog("Invalid parameters in cbs_process_bill_payment");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_process_bill_payment");
        return false;
    }
    
    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // 1. Get account number from card
    char account_number[25] = {0};
    char query[500];
    
    sprintf(query, 
            "SELECT account_number FROM cbs_cards "
            "WHERE card_number = '%s' AND status = 'ACTIVE'", 
            card_number);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query card: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row || !row[0]) {
        writeErrorLog("Card not found or inactive: %s", card_number);
        mysql_free_result(result);
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    strncpy(account_number, row[0], sizeof(account_number) - 1);
    mysql_free_result(result);
    
    // 2. Get current account balance
    double current_balance = 0.0;
    
    sprintf(query, 
            "SELECT balance FROM cbs_accounts "
            "WHERE account_number = '%s' AND status = 'ACTIVE'", 
            account_number);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query account balance: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    row = mysql_fetch_row(result);
    if (!row || !row[0]) {
        writeErrorLog("Account not found or inactive: %s", account_number);
        mysql_free_result(result);
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    current_balance = atof(row[0]);
    mysql_free_result(result);
    
    // 3. Check if sufficient balance
    if (current_balance < amount) {
        writeErrorLog("Insufficient balance for bill payment: %.2f < %.2f", current_balance, amount);
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // 4. Calculate new balance
    double new_balance = current_balance - amount;
    
    // 5. Update account balance
    sprintf(query, 
            "UPDATE cbs_accounts SET balance = %.2f "
            "WHERE account_number = '%s'",
            new_balance, account_number);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to update balance: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // 6. Generate transaction ID
    char transaction_id[37] = {0};
    generate_transaction_id(transaction_id, sizeof(transaction_id));
    
    // 7. Insert bill payment record
    char remarks[200];
    sprintf(remarks, "%s Bill Payment - Ref: %s", bill_type, bill_reference);
    
    // Get current timestamp
    time_t now;
    time(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    sprintf(query,
            "INSERT INTO cbs_transactions "
            "(transaction_id, account_number, transaction_type, amount, "
            "balance_before, balance_after, status, remarks, transaction_date) "
            "VALUES ('%s', '%s', 'BILL_PAYMENT', %.2f, %.2f, %.2f, 'SUCCESS', '%s', '%s')",
            transaction_id, account_number, amount, current_balance, new_balance, remarks, timestamp);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to insert transaction record: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // 8. Insert into bill payments table
    sprintf(query,
            "INSERT INTO cbs_bill_payments "
            "(transaction_id, bill_type, bill_reference, amount, payment_date, card_number) "
            "VALUES ('%s', '%s', '%s', %.2f, '%s', '%s')",
            transaction_id, bill_type, bill_reference, amount, timestamp, card_number);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to insert bill payment record: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // 9. Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0) {
        writeErrorLog("Failed to commit transaction: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Return transaction ID
    strncpy(transaction_id_out, transaction_id, 36);
    transaction_id_out[36] = '\0';
    
    db_release_connection(conn);
    return true;
}

/**
 * @brief Get bill payment history for a card
 * @param card_number Card number
 * @param records Array to store payment records
 * @param count Output parameter for number of records
 * @param max_records Maximum number of records to retrieve
 * @return true if retrieval was successful, false otherwise
 */
bool cbs_get_bill_payment_history(const char* card_number,
                                 TransactionRecord* records,
                                 int* count,
                                 int max_records) {
    if (!card_number || !records || !count || max_records <= 0) {
        writeErrorLog("Invalid parameters in cbs_get_bill_payment_history");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_bill_payment_history");
        return false;
    }
    
    char query[500];
    sprintf(query,
            "SELECT t.transaction_id, b.bill_type, t.amount, t.balance_after, "
            "t.transaction_date, t.status "
            "FROM cbs_transactions t "
            "JOIN cbs_bill_payments b ON t.transaction_id = b.transaction_id "
            "JOIN cbs_accounts a ON t.account_number = a.account_number "
            "JOIN cbs_cards c ON a.account_number = c.account_number "
            "WHERE c.card_number = '%s' "
            "ORDER BY t.transaction_date DESC "
            "LIMIT %d",
            card_number, max_records);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query bill payment history: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    *count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) && (*count < max_records)) {
        if (row[0] && row[1] && row[2] && row[3] && row[4] && row[5]) {
            strncpy(records[*count].transaction_id, row[0], 36);
            records[*count].transaction_id[36] = '\0';
            
            snprintf(records[*count].transaction_type, sizeof(records[*count].transaction_type),
                    "%s Bill", row[1]);
            
            records[*count].amount = atof(row[2]);
            records[*count].balance = atof(row[3]);
            
            strncpy(records[*count].date, row[4], 19);
            records[*count].date[19] = '\0';
            
            bool status = (strcmp(row[5], "SUCCESS") == 0);
            strcpy(records[*count].status, status ? "Success" : "Failed");
            
            (*count)++;
        }
    }
    
    mysql_free_result(result);
    db_release_connection(conn);
    
    return true;
}
