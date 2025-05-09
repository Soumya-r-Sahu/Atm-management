#include "../../../include/common/database/core_banking_interface.h"
#include "../../../include/common/database/db_config.h"
#include "../../../include/common/utils/logger.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Forward declarations
extern MYSQL* db_get_connection();
extern void db_release_connection(MYSQL* conn);

// Helper function to generate UUID
static void generate_uuid(char *uuid_str) {
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid_str[i] = '-';
        } else {
            int r = rand() % 16;
            uuid_str[i] = hex_chars[r];
        }
    }
    uuid_str[36] = '\0';
}

/**
 * @brief Process a transaction with optimized database access
 * Time complexity: O(1) - constant time operation
 */
bool cbs_process_transaction(const char* account_number, 
                            TRANSACTION_TYPE type, 
                            const char* channel, 
                            double amount, 
                            char* transaction_id_out) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_process_transaction");
        return false;
    }
    
    // Generate transaction ID (UUID format)
    char transaction_id[37];
    generate_uuid(transaction_id);
    
    // Get current account balance with a single query
    char balance_query[200];
    sprintf(balance_query, 
            "SELECT balance FROM cbs_accounts WHERE account_number = '%s' FOR UPDATE", 
            account_number);
    
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    double balance_before = 0;
    double balance_after = 0;
    bool success = false;
    
    if (mysql_query(conn, balance_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                balance_before = atof(row[0]);
                
                // Calculate new balance based on transaction type
                balance_after = balance_before;
                if (type == TRANSACTION_WITHDRAWAL || type == TRANSACTION_TRANSFER || 
                    type == TRANSACTION_PAYMENT) {
                    balance_after -= amount;
                } else if (type == TRANSACTION_DEPOSIT || type == TRANSACTION_INTEREST_CREDIT) {
                    balance_after += amount;
                }
                
                // Check for sufficient balance
                if ((type == TRANSACTION_WITHDRAWAL || type == TRANSACTION_TRANSFER || 
                     type == TRANSACTION_PAYMENT) && balance_after < 0) {
                    writeErrorLog("Insufficient funds for account %s", account_number);
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
                
                // Update account balance
                char update_query[300];
                sprintf(update_query, 
                        "UPDATE cbs_accounts SET balance = %.2f, last_transaction = NOW() "
                        "WHERE account_number = '%s'", 
                        balance_after, account_number);
                
                if (mysql_query(conn, update_query) != 0) {
                    writeErrorLog("Failed to update account balance: %s", mysql_error(conn));
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
                
                // Insert transaction record
                char insert_query[1000];
                char transaction_type_str[30];
                
                // Map transaction type to enum string
                switch (type) {
                    case TRANSACTION_WITHDRAWAL: strcpy(transaction_type_str, "WITHDRAWAL"); break;
                    case TRANSACTION_DEPOSIT: strcpy(transaction_type_str, "DEPOSIT"); break;
                    case TRANSACTION_TRANSFER: strcpy(transaction_type_str, "TRANSFER"); break;
                    case TRANSACTION_PAYMENT: strcpy(transaction_type_str, "PAYMENT"); break;
                    case TRANSACTION_BALANCE_INQUIRY: strcpy(transaction_type_str, "BALANCE_INQUIRY"); break;
                    case TRANSACTION_MINI_STATEMENT: strcpy(transaction_type_str, "MINI_STATEMENT"); break;
                    case TRANSACTION_PIN_CHANGE: strcpy(transaction_type_str, "PIN_CHANGE"); break;
                    case TRANSACTION_INTEREST_CREDIT: strcpy(transaction_type_str, "INTEREST_CREDIT"); break;
                    default: strcpy(transaction_type_str, "OTHER");
                }
                
                sprintf(insert_query,
                        "INSERT INTO cbs_transactions (transaction_id, account_number, transaction_type, "
                        "channel, amount, balance_before, balance_after, transaction_date, value_date, status) "
                        "VALUES ('%s', '%s', '%s', '%s', %.2f, %.2f, %.2f, NOW(), CURDATE(), 'SUCCESS')",
                        transaction_id, account_number, transaction_type_str, channel, 
                        amount, balance_before, balance_after);
                
                if (mysql_query(conn, insert_query) != 0) {
                    writeErrorLog("Failed to insert transaction record: %s", mysql_error(conn));
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
                
                success = true;
                
                if (transaction_id_out) {
                    strcpy(transaction_id_out, transaction_id);
                }
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to get account balance: %s", mysql_error(conn));
    }
    
    if (success) {
        mysql_query(conn, "COMMIT");
    } else {
        mysql_query(conn, "ROLLBACK");
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Get account balance with minimum overhead
 * Time complexity: O(1) - constant time database lookup
 */
bool cbs_get_account_balance(const char* account_number, double* balance_out) {
    if (!account_number || !balance_out) {
        writeErrorLog("Invalid parameters in cbs_get_account_balance");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_account_balance");
        return false;
    }
    
    char query[200];
    sprintf(query, 
            "SELECT balance FROM cbs_accounts WHERE account_number = '%s'",
            account_number);
    
    bool success = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                *balance_out = atof(row[0]);
                success = true;
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to get account balance: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Process fund transfer between accounts with optimized implementation
 * Time complexity: O(1) - constant time operations
 */
bool cbs_transfer_funds(const char* source_account, 
                       const char* destination_account, 
                       double amount, 
                       const char* transfer_type,
                       char* transaction_id_out) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_transfer_funds");
        return false;
    }
    
    // Generate transaction and transfer IDs
    char transaction_id[37];
    char transfer_id[37];
    generate_uuid(transaction_id);
    generate_uuid(transfer_id);
    
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Get source account balance
    char source_query[200];
    sprintf(source_query, 
            "SELECT balance, status FROM cbs_accounts WHERE account_number = '%s' FOR UPDATE",
            source_account);
    
    double source_balance = 0;
    char source_status[20] = "";
    bool success = false;
    
    if (mysql_query(conn, source_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0] && row[1]) {
                source_balance = atof(row[0]);
                strcpy(source_status, row[1]);
                
                // Check source account status
                if (strcmp(source_status, "ACTIVE") != 0) {
                    writeErrorLog("Source account %s is not active", source_account);
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
                
                // Check sufficient balance
                if (source_balance < amount) {
                    writeErrorLog("Insufficient funds in source account %s", source_account);
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to get source account balance: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Check destination account status if it's an internal transfer
    if (strcmp(transfer_type, "INTERNAL") == 0) {
        char dest_query[200];
        sprintf(dest_query, 
                "SELECT status FROM cbs_accounts WHERE account_number = '%s' FOR UPDATE",
                destination_account);
        
        if (mysql_query(conn, dest_query) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result) {
                MYSQL_ROW row = mysql_fetch_row(result);
                if (row && row[0]) {
                    if (strcmp(row[0], "ACTIVE") != 0) {
                        writeErrorLog("Destination account %s is not active", destination_account);
                        mysql_free_result(result);
                        mysql_query(conn, "ROLLBACK");
                        db_release_connection(conn);
                        return false;
                    }
                } else {
                    writeErrorLog("Destination account %s not found", destination_account);
                    mysql_free_result(result);
                    mysql_query(conn, "ROLLBACK");
                    db_release_connection(conn);
                    return false;
                }
                mysql_free_result(result);
            }
        } else {
            writeErrorLog("Failed to get destination account status: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            db_release_connection(conn);
            return false;
        }
    }
    
    // Update source account balance
    double source_balance_after = source_balance - amount;
    char update_source_query[300];
    sprintf(update_source_query,
            "UPDATE cbs_accounts SET balance = %.2f, last_transaction = NOW() "
            "WHERE account_number = '%s'",
            source_balance_after, source_account);
    
    if (mysql_query(conn, update_source_query) != 0) {
        writeErrorLog("Failed to update source account balance: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Insert transaction record
    char insert_transaction_query[1000];
    sprintf(insert_transaction_query,
            "INSERT INTO cbs_transactions (transaction_id, account_number, transaction_type, "
            "channel, amount, balance_before, balance_after, transaction_date, value_date, status) "
            "VALUES ('%s', '%s', 'TRANSFER', 'ONLINE', %.2f, %.2f, %.2f, NOW(), CURDATE(), 'SUCCESS')",
            transaction_id, source_account, amount, source_balance, source_balance_after);
    
    if (mysql_query(conn, insert_transaction_query) != 0) {
        writeErrorLog("Failed to insert transaction record: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // For internal transfers, update destination account balance
    if (strcmp(transfer_type, "INTERNAL") == 0) {
        // Get destination account balance
        char dest_balance_query[200];
        sprintf(dest_balance_query,
                "SELECT balance FROM cbs_accounts WHERE account_number = '%s' FOR UPDATE",
                destination_account);
        
        double dest_balance = 0;
        if (mysql_query(conn, dest_balance_query) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result) {
                MYSQL_ROW row = mysql_fetch_row(result);
                if (row && row[0]) {
                    dest_balance = atof(row[0]);
                }
                mysql_free_result(result);
            }
        } else {
            writeErrorLog("Failed to get destination account balance: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            db_release_connection(conn);
            return false;
        }
        
        // Update destination account balance
        double dest_balance_after = dest_balance + amount;
        char update_dest_query[300];
        sprintf(update_dest_query,
                "UPDATE cbs_accounts SET balance = %.2f, last_transaction = NOW() "
                "WHERE account_number = '%s'",
                dest_balance_after, destination_account);
        
        if (mysql_query(conn, update_dest_query) != 0) {
            writeErrorLog("Failed to update destination account balance: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            db_release_connection(conn);
            return false;
        }
        
        // Insert credit transaction for destination
        char dest_transaction_id[37];
        generate_uuid(dest_transaction_id);
        
        char insert_dest_transaction[1000];
        sprintf(insert_dest_transaction,
                "INSERT INTO cbs_transactions (transaction_id, account_number, transaction_type, "
                "channel, amount, balance_before, balance_after, transaction_date, value_date, status) "
                "VALUES ('%s', '%s', 'DEPOSIT', 'ONLINE', %.2f, %.2f, %.2f, NOW(), CURDATE(), 'SUCCESS')",
                dest_transaction_id, destination_account, amount, dest_balance, dest_balance_after);
        
        if (mysql_query(conn, insert_dest_transaction) != 0) {
            writeErrorLog("Failed to insert destination transaction: %s", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            db_release_connection(conn);
            return false;
        }
    }
    
    // Insert transfer record
    char insert_transfer_query[1000];
    sprintf(insert_transfer_query,
            "INSERT INTO cbs_transfers (transfer_id, transaction_id, source_account, "
            "destination_account, transfer_type, amount, transfer_date, status) "
            "VALUES ('%s', '%s', '%s', '%s', '%s', %.2f, NOW(), 'SUCCESS')",
            transfer_id, transaction_id, source_account, destination_account, 
            transfer_type, amount);
    
    if (mysql_query(conn, insert_transfer_query) != 0) {
        writeErrorLog("Failed to insert transfer record: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    // Commit transaction
    if (mysql_query(conn, "COMMIT") != 0) {
        writeErrorLog("Failed to commit transaction: %s", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        return false;
    }
    
    if (transaction_id_out) {
        strcpy(transaction_id_out, transaction_id);
    }
    
    db_release_connection(conn);
    return true;
}

/**
 * @brief Get mini statement with transaction history
 * Time complexity: O(n) where n is the number of transactions retrieved (limited by LIMIT clause)
 */
bool cbs_get_mini_statement(const char* account_number, TransactionRecord* records, int* count, int max_records) {
    if (!account_number || !records || !count || max_records <= 0) {
        writeErrorLog("Invalid parameters in cbs_get_mini_statement");
        return false;
    }
    
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_get_mini_statement");
        return false;
    }
    
    char query[500];
    sprintf(query,
            "SELECT transaction_id, transaction_type, amount, balance_after, "
            "transaction_date, status FROM cbs_transactions "
            "WHERE account_number = '%s' "
            "ORDER BY transaction_date DESC LIMIT %d",
            account_number, max_records);
    
    bool success = false;
    *count = 0;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) && *count < max_records) {
                if (row[0] && row[1] && row[2] && row[3] && row[4] && row[5]) {
                    strncpy(records[*count].transaction_id, row[0], sizeof(records[*count].transaction_id) - 1);
                    strncpy(records[*count].transaction_type, row[1], sizeof(records[*count].transaction_type) - 1);
                    records[*count].amount = atof(row[2]);
                    records[*count].balance = atof(row[3]);
                    strncpy(records[*count].date, row[4], sizeof(records[*count].date) - 1);
                    strncpy(records[*count].status, row[5], sizeof(records[*count].status) - 1);
                    
                    records[*count].transaction_id[sizeof(records[*count].transaction_id) - 1] = '\0';
                    records[*count].transaction_type[sizeof(records[*count].transaction_type) - 1] = '\0';
                    records[*count].date[sizeof(records[*count].date) - 1] = '\0';
                    records[*count].status[sizeof(records[*count].status) - 1] = '\0';
                    
                    (*count)++;
                }
            }
            mysql_free_result(result);
            success = true;
        }
    } else {
        writeErrorLog("Failed to get mini statement: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Check daily withdrawal limit based on card channel
 * Time complexity: O(1) - constant time operations with indexed queries
 */
bool cbs_check_withdrawal_limit(const char* card_number, 
                               double amount, 
                               const char* channel, 
                               double* remaining_limit) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_check_withdrawal_limit");
        return false;
    }
    
    // Get card's daily limit based on channel
    char limit_query[300];
    const char* limit_column;
    
    if (strcmp(channel, "ATM") == 0) {
        limit_column = "daily_atm_limit";
    } else if (strcmp(channel, "POS") == 0) {
        limit_column = "daily_pos_limit";
    } else {
        limit_column = "daily_online_limit";
    }
    
    sprintf(limit_query,
            "SELECT %s FROM cbs_cards WHERE card_number = '%s'",
            limit_column, card_number);
    
    bool success = false;
    double daily_limit = 0;
    
    if (mysql_query(conn, limit_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) {
                daily_limit = atof(row[0]);
                
                // Get total withdrawals today
                char withdrawals_query[400];
                sprintf(withdrawals_query,
                        "SELECT COALESCE(SUM(amount), 0) "
                        "FROM cbs_daily_withdrawals "
                        "WHERE card_number = '%s' AND withdrawal_date = CURDATE()",
                        card_number);
                
                mysql_free_result(result);
                
                if (mysql_query(conn, withdrawals_query) == 0) {
                    result = mysql_store_result(conn);
                    if (result) {
                        row = mysql_fetch_row(result);
                        if (row) {
                            double total_today = row[0] ? atof(row[0]) : 0;
                            *remaining_limit = daily_limit - total_today;
                            
                            // Check if withdrawal is allowed
                            if ((total_today + amount) <= daily_limit) {
                                success = true;
                            }
                        }
                        mysql_free_result(result);
                    }
                }
            }
        }
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Process a fixed deposit creation with optimized implementation
 * Time complexity: O(1) - constant time operations
 */
bool cbs_create_fixed_deposit(const char* account_number, 
                              const char* customer_id,
                              double principal_amount, 
                              double interest_rate,
                              int tenure_months,
                              const char* payout_frequency,
                              char* fd_id_out) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_create_fixed_deposit");
        return false;
    }
    
    // Generate FD ID
    char fd_id[37];
    generate_uuid(fd_id);
    
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Calculate maturity date and amount
    char start_date[11];
    char maturity_date[11];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", t);
    
    // Calculate maturity date
    t->tm_mon += tenure_months;
    while (t->tm_mon > 11) {
        t->tm_mon -= 12;
        t->tm_year++;
    }
    strftime(maturity_date, sizeof(maturity_date), "%Y-%m-%d", t);
    
    // Calculate maturity amount (simple interest calculation for efficiency)
    double maturity_amount = principal_amount * (1 + (interest_rate * tenure_months / 1200));
    
    // Insert into fixed deposits table
    char insert_query[1000];
    sprintf(insert_query,
            "INSERT INTO cbs_fixed_deposits (fd_id, account_number, customer_id, "
            "principal_amount, interest_rate, tenure_months, start_date, maturity_date, "
            "maturity_amount, interest_payout_frequency) "
            "VALUES ('%s', '%s', '%s', %.2f, %.2f, %d, '%s', '%s', %.2f, '%s')",
            fd_id, account_number, customer_id, principal_amount, interest_rate, 
            tenure_months, start_date, maturity_date, maturity_amount, payout_frequency);
    
    bool success = false;
    
    if (mysql_query(conn, insert_query) == 0) {
        // Debit from the main account
        char debit_query[300];
        sprintf(debit_query,
                "UPDATE cbs_accounts SET balance = balance - %.2f "
                "WHERE account_number = '%s' AND balance >= %.2f",
                principal_amount, account_number, principal_amount);
        
        if (mysql_query(conn, debit_query) == 0) {
            if (mysql_affected_rows(conn) > 0) {
                // Generate transaction record
                char transaction_id[37];
                generate_uuid(transaction_id);
                
                char transaction_query[1000];
                sprintf(transaction_query,
                        "INSERT INTO cbs_transactions (transaction_id, account_number, "
                        "transaction_type, channel, amount, transaction_date, value_date, status, remarks) "
                        "VALUES ('%s', '%s', 'WITHDRAWAL', 'BRANCH', %.2f, NOW(), CURDATE(), 'SUCCESS', "
                        "'Fixed deposit creation - FD ID: %s')",
                        transaction_id, account_number, principal_amount, fd_id);
                
                if (mysql_query(conn, transaction_query) == 0) {
                    success = true;
                    if (fd_id_out) {
                        strcpy(fd_id_out, fd_id);
                    }
                } else {
                    writeErrorLog("Failed to create transaction record: %s", mysql_error(conn));
                }
            } else {
                writeErrorLog("Insufficient balance for FD creation");
            }
        } else {
            writeErrorLog("Failed to debit account: %s", mysql_error(conn));
        }
    } else {
        writeErrorLog("Failed to create fixed deposit: %s", mysql_error(conn));
    }
    
    if (success) {
        mysql_query(conn, "COMMIT");
    } else {
        mysql_query(conn, "ROLLBACK");
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Process loan EMI payment with optimized implementation
 * Time complexity: O(1) - constant time operations
 */
bool cbs_process_loan_emi(const char* loan_id,
                          double emi_amount,
                          double principal_component,
                          double interest_component,
                          const char* payment_mode) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_process_loan_emi");
        return false;
    }
    
    // Generate repayment ID
    char repayment_id[37];
    generate_uuid(repayment_id);
    
    if (mysql_query(conn, "START TRANSACTION") != 0) {
        writeErrorLog("Failed to start transaction: %s", mysql_error(conn));
        db_release_connection(conn);
        return false;
    }
    
    // Get loan details
    char loan_query[200];
    sprintf(loan_query,
            "SELECT account_number, emis_paid, total_emis "
            "FROM cbs_loans WHERE loan_id = '%s' FOR UPDATE",
            loan_id);
    
    bool success = false;
    char account_number[21] = "";
    int emis_paid = 0;
    int total_emis = 0;
    
    if (mysql_query(conn, loan_query) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0] && row[1] && row[2]) {
                strncpy(account_number, row[0], sizeof(account_number) - 1);
                account_number[sizeof(account_number) - 1] = '\0';
                emis_paid = atoi(row[1]);
                total_emis = atoi(row[2]);
                
                // Insert repayment record
                char insert_query[1000];
                sprintf(insert_query,
                        "INSERT INTO cbs_loan_repayments (repayment_id, loan_id, amount, "
                        "principal_component, interest_component, repayment_date, payment_mode, status) "
                        "VALUES ('%s', '%s', %.2f, %.2f, %.2f, CURDATE(), '%s', 'PAID')",
                        repayment_id, loan_id, emi_amount, principal_component, 
                        interest_component, payment_mode);
                
                if (mysql_query(conn, insert_query) == 0) {
                    // Update loan EMIs paid
                    char update_loan_query[300];
                    sprintf(update_loan_query,
                            "UPDATE cbs_loans SET emis_paid = emis_paid + 1 WHERE loan_id = '%s'",
                            loan_id);
                    
                    if (mysql_query(conn, update_loan_query) == 0) {
                        // Check if loan is fully paid
                        if (emis_paid + 1 >= total_emis) {
                            // Update loan status to CLOSED
                            char close_loan_query[200];
                            sprintf(close_loan_query,
                                    "UPDATE cbs_loans SET status = 'CLOSED' WHERE loan_id = '%s'",
                                    loan_id);
                            
                            if (mysql_query(conn, close_loan_query) != 0) {
                                writeErrorLog("Failed to close loan: %s", mysql_error(conn));
                            }
                        }
                        
                        success = true;
                    } else {
                        writeErrorLog("Failed to update loan EMIs paid: %s", mysql_error(conn));
                    }
                } else {
                    writeErrorLog("Failed to insert repayment record: %s", mysql_error(conn));
                }
            }
            mysql_free_result(result);
        }
    } else {
        writeErrorLog("Failed to get loan details: %s", mysql_error(conn));
    }
    
    if (success) {
        mysql_query(conn, "COMMIT");
    } else {
        mysql_query(conn, "ROLLBACK");
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Update card limits with optimized implementation
 * Time complexity: O(1) - constant time operation
 */
bool cbs_update_card_limits(const char* card_number, 
                           double atm_limit, 
                           double pos_limit, 
                           double online_limit) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_update_card_limits");
        return false;
    }
    
    char update_query[400];
    sprintf(update_query,
            "UPDATE cbs_cards SET "
            "daily_atm_limit = %.2f, "
            "daily_pos_limit = %.2f, "
            "daily_online_limit = %.2f "
            "WHERE card_number = '%s'",
            atm_limit, pos_limit, online_limit, card_number);
    
    bool success = false;
    
    if (mysql_query(conn, update_query) == 0) {
        if (mysql_affected_rows(conn) > 0) {
            success = true;
            
            // Log to audit log
            char audit_query[500];
            sprintf(audit_query,
                    "INSERT INTO cbs_audit_logs (action, entity_type, entity_id, details) "
                    "VALUES ('LIMIT_CHANGE', 'CARD', '%s', "
                    "'Card limits updated: ATM=%.2f, POS=%.2f, Online=%.2f')",
                    card_number, atm_limit, pos_limit, online_limit);
            
            if (mysql_query(conn, audit_query) != 0) {
                writeErrorLog("Failed to log limit change in audit: %s", mysql_error(conn));
            }
        }
    } else {
        writeErrorLog("Failed to update card limits: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

/**
 * @brief Create a new beneficiary with optimized implementation
 * Time complexity: O(1) - constant time operation
 */
bool cbs_add_beneficiary(const char* customer_id,
                        const char* beneficiary_name,
                        const char* account_number,
                        const char* ifsc_code,
                        const char* bank_name,
                        const char* account_type) {
    MYSQL *conn = db_get_connection();
    if (!conn) {
        writeErrorLog("Failed to get database connection in cbs_add_beneficiary");
        return false;
    }
    
    // Generate beneficiary ID
    char beneficiary_id[37];
    generate_uuid(beneficiary_id);
    
    char insert_query[1000];
    sprintf(insert_query,
            "INSERT INTO cbs_beneficiaries (beneficiary_id, customer_id, beneficiary_name, "
            "account_number, ifsc_code, bank_name, account_type) "
            "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s')",
            beneficiary_id, customer_id, beneficiary_name, account_number, 
            ifsc_code, bank_name, account_type);
    
    bool success = false;
    
    if (mysql_query(conn, insert_query) == 0) {
        success = true;
        
        // Log to audit log
        char audit_query[500];
        sprintf(audit_query,
                "INSERT INTO cbs_audit_logs (action, entity_type, entity_id, user_id, details) "
                "VALUES ('BENEFICIARY_ADD', 'BENEFICIARY', '%s', '%s', "
                "'Added beneficiary %s with account %s')",
                beneficiary_id, customer_id, beneficiary_name, account_number);
        
        if (mysql_query(conn, audit_query) != 0) {
            writeErrorLog("Failed to log beneficiary addition: %s", mysql_error(conn));
        }
    } else {
        writeErrorLog("Failed to add beneficiary: %s", mysql_error(conn));
    }
    
    db_release_connection(conn);
    return success;
}

