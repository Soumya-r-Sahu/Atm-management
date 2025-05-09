#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/**
 * Update daily withdrawal tracking after a successful withdrawal
 * This function creates or updates a record in the daily withdrawals table
 */
bool cbs_update_daily_withdrawal(const char* card_number, double amount) {
    if (!card_number || amount <= 0) {
        writeErrorLog("Invalid parameters in cbs_update_daily_withdrawal");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_update_daily_withdrawal");
        return false;
    }
    
    // Check if a record exists for today
    char check_query[300];
    sprintf(check_query, 
            "SELECT withdrawal_id, amount FROM cbs_daily_withdrawals "
            "WHERE card_number = '%s' AND withdrawal_date = CURDATE()", 
            card_number);
    
    if (mysql_query(conn, check_query) != 0) {
        writeErrorLog("Failed to check daily withdrawals: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    bool success = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    
    if (row && row[0]) {
        // Record exists, update amount
        char update_query[400];
        double current_amount = atof(row[1]);
        double new_amount = current_amount + amount;
        
        sprintf(update_query, 
                "UPDATE cbs_daily_withdrawals "
                "SET amount = %.2f, last_updated = NOW() "
                "WHERE withdrawal_id = '%s'", 
                new_amount, row[0]);
        
        mysql_free_result(result);
        
        if (mysql_query(conn, update_query) == 0) {
            success = true;
            writeInfoLog("Updated daily withdrawal tracking for card %s: total now %.2f", 
                        card_number, new_amount);
        } else {
            writeErrorLog("Failed to update daily withdrawal: %s", mysql_error(conn));
        }
    } else {
        // No record for today, create new one
        char insert_query[400];
        sprintf(insert_query, 
                "INSERT INTO cbs_daily_withdrawals "
                "(card_number, withdrawal_date, amount, status, last_updated) "
                "VALUES ('%s', CURDATE(), %.2f, 'COMPLETED', NOW())", 
                card_number, amount);
        
        mysql_free_result(result);
        
        if (mysql_query(conn, insert_query) == 0) {
            success = true;
            writeInfoLog("Created daily withdrawal tracking for card %s: %.2f", 
                        card_number, amount);
        } else {
            writeErrorLog("Failed to create daily withdrawal record: %s", mysql_error(conn));
        }
    }
    
    db_release_connection(conn);
    return success;
}
