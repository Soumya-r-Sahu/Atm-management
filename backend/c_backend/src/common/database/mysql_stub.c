/**
 * @file mysql_stub.c
 * @brief Stub implementation for MySQL when not available
 */

#include "../../include/common/utils/logger.h"
#include "../../include/common/database/mysql_stub.h"

#ifdef NO_MYSQL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stub structure definitions
struct MYSQL {
    char *last_error;
    int connected;
};

struct MYSQL_RES {
    int rows;
    int fields;
    char ***data;  // Simulated result data
    int current_row;
    char **current_row_data;  // Current row data to return
};

struct MYSQL_FIELD {
    char *name;
};

// Global MySQL instance for stub
static MYSQL global_mysql_instance = {NULL, 0};

// Stub implementations

MYSQL *mysql_init(MYSQL *mysql) {
    log_info("STUB: mysql_init called");
    
    if (mysql == NULL) {
        mysql = &global_mysql_instance;
    }
    
    mysql->last_error = NULL;
    mysql->connected = 0;
    
    return mysql;
}

MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, 
                         const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag) {
    log_info("STUB: mysql_real_connect called for host '%s', user '%s', database '%s'", 
             host ? host : "NULL", user ? user : "NULL", db ? db : "NULL");
    
    if (mysql == NULL) {
        return NULL;
    }
    
    mysql->connected = 1;
    mysql->last_error = NULL;
    
    return mysql;
}

// Global state for SQL test harness stub
static struct {
    int card_number;
    int account_number;
    int pin;
    float balance;
    char customer_name[100];
    char status[20];
    bool card_exists;
} mysql_stub_harness_state = {
    123456789,  // card_number
    10001,      // account_number
    1234,       // pin
    1500.0,     // balance
    "John Smith", // customer_name
    "active",   // status
    true        // card_exists
};

int mysql_query(MYSQL *mysql, const char *query) {
    log_info("STUB: mysql_query called with query: %s", query ? query : "NULL");
    
    if (mysql == NULL || !mysql->connected) {
        return 1; // Error
    }
    
    // Handle specific query types for the test harness
    if (query) {
        if (strstr(query, "UPDATE") && strstr(query, "balance")) {
            // Handle balance updates
            if (strstr(query, "-")) {
                // Withdrawal
                mysql_stub_harness_state.balance -= 100.0; // Assume standard amount
            } else {
                // Deposit
                mysql_stub_harness_state.balance += 100.0; // Assume standard amount
            }
        }
    }
    
    return 0; // Success
}

MYSQL_RES *mysql_store_result(MYSQL *mysql) {
    log_info("STUB: mysql_store_result called");
    
    if (mysql == NULL || !mysql->connected) {
        return NULL;
    }
    
    MYSQL_RES *result = (MYSQL_RES *)malloc(sizeof(MYSQL_RES));
    if (result) {
        result->rows = mysql_stub_harness_state.card_exists ? 1 : 0;  // 1 row if card exists
        result->fields = 5;  // Fixed number of fields for our stub
        result->data = NULL;
        result->current_row = 0;
        result->current_row_data = NULL;
    }
    
    return result;
}

MYSQL_ROW mysql_fetch_row(MYSQL_RES *result) {
    log_info("STUB: mysql_fetch_row called");
    
    if (result == NULL || result->current_row >= result->rows) {
        return NULL;
    }
    
    // Free any previous row data
    if (result->current_row_data) {
        int i;
        for (i = 0; i < result->fields; i++) {
            if (result->current_row_data[i]) {
                free(result->current_row_data[i]);
            }
        }
        free(result->current_row_data);
    }
    
    // Allocate and populate the row based on mysql_stub_harness_state
    result->current_row_data = (char**)malloc(sizeof(char*) * 5); // Up to 5 fields
    
    if (!result->current_row_data) {
        return NULL;
    }
    
    // Card number field
    result->current_row_data[0] = (char*)malloc(20);
    if (result->current_row_data[0]) {
        sprintf(result->current_row_data[0], "%d", mysql_stub_harness_state.card_number);
    }
    
    // Account number field
    result->current_row_data[1] = (char*)malloc(20);
    if (result->current_row_data[1]) {
        sprintf(result->current_row_data[1], "%d", mysql_stub_harness_state.account_number);
    }
    
    // Customer name field
    result->current_row_data[2] = (char*)malloc(100);
    if (result->current_row_data[2]) {
        strcpy(result->current_row_data[2], mysql_stub_harness_state.customer_name);
    }
    
    // Balance field
    result->current_row_data[3] = (char*)malloc(20);
    if (result->current_row_data[3]) {
        sprintf(result->current_row_data[3], "%.2f", mysql_stub_harness_state.balance);
    }
    
    // Status field
    result->current_row_data[4] = (char*)malloc(20);
    if (result->current_row_data[4]) {
        strcpy(result->current_row_data[4], mysql_stub_harness_state.status);
    }
    
    result->current_row++;
    return result->current_row_data;
}

unsigned int mysql_num_fields(MYSQL_RES *result) {
    log_info("STUB: mysql_num_fields called");
    
    if (result == NULL) {
        return 0;
    }
    
    return result->fields;
}

unsigned long mysql_num_rows(MYSQL_RES *result) {
    log_info("STUB: mysql_num_rows called");
    
    if (result == NULL) {
        return 0;
    }
    
    return result->rows;
}

void mysql_free_result(MYSQL_RES *result) {
    log_info("STUB: mysql_free_result called");
    
    if (result) {
        // Free any current row data
        if (result->current_row_data) {
            int i;
            for (i = 0; i < result->fields; i++) {
                if (result->current_row_data[i]) {
                    free(result->current_row_data[i]);
                }
            }
            free(result->current_row_data);
        }
        
        // Free the result structure itself
        free(result);
    }
}

void mysql_close(MYSQL *mysql) {
    log_info("STUB: mysql_close called");
    
    if (mysql) {
        mysql->connected = 0;
    }
}

const char *mysql_error(MYSQL *mysql) {
    log_info("STUB: mysql_error called");
    
    if (mysql == NULL) {
        return "MySQL instance is NULL";
    }
    
    if (mysql->last_error == NULL) {
        return "No error information available";
    }
    
    return mysql->last_error;
}

#endif // NO_MYSQL
