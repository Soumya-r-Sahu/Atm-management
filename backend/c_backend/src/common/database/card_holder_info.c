#include "../../../include/common/database/card_account_management.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/database/database_conn.h"
#include "../../../include/common/utils/logger.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Get cardholder name from card number
 */
bool cbs_get_card_holder_name(const char* card_number, char* name_out, size_t name_out_size) {
    if (!card_number || !name_out || name_out_size <= 0) {
        writeErrorLog("Invalid parameters in cbs_get_card_holder_name");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_card_holder_name");
        return false;
    }
    
    char query[500];
    sprintf(query, 
            "SELECT c.first_name, c.last_name FROM cbs_customers c "
            "JOIN cbs_accounts a ON c.customer_id = a.customer_id "
            "JOIN cbs_cards cd ON a.account_number = cd.account_number "
            "WHERE cd.card_number = '%s'", 
            card_number);
    
    if (mysql_query(conn, query) != 0) {
        writeErrorLog("Failed to query cardholder name: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        writeErrorLog("Failed to store result: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    bool success = false;
    
    if (row && row[0] && row[1]) {
        // Format the name as "First Last"
        snprintf(name_out, name_out_size, "%s %s", row[0], row[1]);
        success = true;
    } else {
        // If customer not found, set to "Unknown"
        snprintf(name_out, name_out_size, "Unknown Customer");
    }
    
    mysql_free_result(result);
    db_release_connection(conn);
    
    return success;
}
