/**
 * @file transaction_history.c
 * @brief Implementation of transaction history retrieval for Core Banking System
 */

#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Get transaction history for an account
 * @param account_number Account number
 * @param records Array to store transaction records
 * @param count Output parameter for number of records found
 * @param max_records Maximum number of records to retrieve
 * @return true if successful, false otherwise
 */
bool cbs_get_transaction_history(const char* account_number, 
                                TransactionRecord* records, 
                                int* count, 
                                int max_records) {
    if (!account_number || !records || !count || max_records <= 0) {
        writeErrorLog("Invalid parameters in cbs_get_transaction_history");
        return false;
    }
    
    // Get MySQL connection
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_transaction_history");
        return false;
    }
    
    char query[500];
    sprintf(query,
            "SELECT transaction_id, transaction_type, amount, balance_after, "
            "transaction_date, status, remarks "
            "FROM cbs_transactions "
            "WHERE account_number = '%s' "
            "ORDER BY transaction_date DESC "
            "LIMIT %d",
            account_number, max_records);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query transaction history: %s", mysql_error(conn));
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
            // Transaction ID
            strncpy(records[*count].transaction_id, row[0], 36);
            records[*count].transaction_id[36] = '\0';
            
            // Transaction type
            strncpy(records[*count].transaction_type, row[1], 29);
            records[*count].transaction_type[29] = '\0';
            
            // Amount and balance
            records[*count].amount = atof(row[2]);
            records[*count].balance = atof(row[3]);
            
            // Date
            strncpy(records[*count].date, row[4], 19);
            records[*count].date[19] = '\0';
            
            // Status
            strncpy(records[*count].status, row[5], 19);
            records[*count].status[19] = '\0';
            
            (*count)++;
        }
    }
    
    mysql_free_result(result);
    db_release_connection(conn);
    
    return true;
}
