#include "atm/transaction/transaction_processor.h"
#include "common/database/database.h"
#include "common/database/db_config.h"
#include "common/database/database_compat.h"
#include "common/database/db_constants.h"  // Added database constants include
#include "common/utils/logger.h"
#include "common/utils/hash_utils.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Process balance inquiry
TransactionResult process_balance_inquiry(int card_number) {
    TransactionResult result = {0};
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get the balance
    float balance = 0.0f;
    if (!fetchBalance(card_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = 0;
    result.balance_before = balance;
    result.balance_after = balance;
    sprintf(result.message, "Current balance: $%.2f", balance);
    
    // Log the transaction
    logTransaction(card_number, "Balance", 0, true);
    
    return result;
}

// Process withdrawal
TransactionResult process_withdrawal(int card_number, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid withdrawal amount");
        return result;
    }
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = 0.0f;
    if (!fetchBalance(card_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds");
        return result;
    }
    
    // Get daily withdrawal total and check if it exceeds limit
    MYSQL *conn = (MYSQL*)db_get_connection();
    if (!conn) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Database connection error");
        return result;
    }
    
    char query[500];
    sprintf(query, "SELECT SUM(amount) FROM %s WHERE cardNumber = %d AND type = 'Withdrawal' "
            "AND DATE(timestamp) = CURDATE() AND status = 'Success'", TABLE_TRANSACTIONS, card_number);
    
    float dailyTotal = 0.0f;
    float dailyLimit = 10000.0f; // Default limit
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result_set = mysql_store_result(conn);
        if (result_set) {
            MYSQL_ROW row = mysql_fetch_row(result_set);
            if (row && row[0]) {
                dailyTotal = atof(row[0]);
            }
            mysql_free_result(result_set);
        }
        
        // Get card's daily withdrawal limit
        sprintf(query, "SELECT dailyWithdrawalLimit FROM %s WHERE cardNumber = %d", 
                TABLE_CARDS, card_number);
        
        if (mysql_query(conn, query) == 0) {
            MYSQL_RES *limit_result = mysql_store_result(conn);
            if (limit_result) {
                MYSQL_ROW limit_row = mysql_fetch_row(limit_result);
                if (limit_row && limit_row[0]) {
                    dailyLimit = atof(limit_row[0]);
                }
                mysql_free_result(limit_result);
            }
        }
    }
    
    db_release_connection(conn);
    
    if (dailyTotal + amount > dailyLimit) {
        result.status = TRANSACTION_FAILED;
        sprintf(result.message, "Daily withdrawal limit (%.2f) exceeded", dailyLimit);
        return result;
    }
    
    // Update balance
    float new_balance = balance - amount;
    if (!updateBalance(card_number, new_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update account balance");
        return result;
    }
    
    // Log the withdrawal
    logWithdrawal(card_number, amount);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_before = balance;
    result.balance_after = new_balance;
    sprintf(result.message, "Withdrawal successful. New balance: $%.2f", new_balance);
    
    // Log the transaction
    logTransaction(card_number, "Withdrawal", amount, true);
    
    return result;
}

// Process deposit
TransactionResult process_deposit(int card_number, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid deposit amount");
        return result;
    }
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = 0.0f;
    if (!fetchBalance(card_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Update balance
    float new_balance = balance + amount;
    if (!updateBalance(card_number, new_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update account balance");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_before = balance;
    result.balance_after = new_balance;
    sprintf(result.message, "Deposit successful. New balance: $%.2f", new_balance);
    
    // Log the transaction
    logTransaction(card_number, "Deposit", amount, true);
    
    return result;
}

// Process transfer
TransactionResult process_transfer(int sender_card, int receiver_card, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid transfer amount");
        return result;
    }
    
    // Check if cards are valid
    if (!doesCardExist(sender_card) || !doesCardExist(receiver_card)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if same card
    if (sender_card == receiver_card) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Cannot transfer to the same card");
        return result;
    }
    
    // Check if cards are active
    if (!isCardActive(sender_card) || !isCardActive(receiver_card)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "One of the cards is inactive or blocked");
        return result;
    }
    
    // Get sender balance
    float sender_balance = 0.0f;
    if (!fetchBalance(sender_card, &sender_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve sender account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (sender_balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds");
        return result;
    }
    
    // Get receiver balance
    float receiver_balance = 0.0f;
    if (!fetchBalance(receiver_card, &receiver_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve receiver account balance");
        return result;
    }
    
    // Get account numbers for both cards
    MYSQL *conn = (MYSQL*)db_get_connection();
    if (!conn) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Database connection error");
        return result;
    }
    
    char sender_account[21] = {0};
    char receiver_account[21] = {0};
    char query[500];
    
    sprintf(query, "SELECT a.accountNumber FROM %s a JOIN %s c ON a.customerId = c.customerId "
            "JOIN %s cd ON c.customerId = cd.customerId WHERE cd.cardNumber = %d", 
            TABLE_ACCOUNTS, TABLE_CUSTOMERS, TABLE_CARDS, sender_card);
    
    bool accounts_found = false;
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES *result_set = mysql_store_result(conn);
        if (result_set) {
            MYSQL_ROW row = mysql_fetch_row(result_set);
            if (row && row[0]) {
                strncpy(sender_account, row[0], sizeof(sender_account) - 1);
                
                // Now get the receiver account
                sprintf(query, "SELECT a.accountNumber FROM %s a JOIN %s c ON a.customerId = c.customerId "
                        "JOIN %s cd ON c.customerId = cd.customerId WHERE cd.cardNumber = %d", 
                        TABLE_ACCOUNTS, TABLE_CUSTOMERS, TABLE_CARDS, receiver_card);
                
                mysql_free_result(result_set);
                
                if (mysql_query(conn, query) == 0) {
                    result_set = mysql_store_result(conn);
                    if (result_set) {
                        row = mysql_fetch_row(result_set);
                        if (row && row[0]) {
                            strncpy(receiver_account, row[0], sizeof(receiver_account) - 1);
                            accounts_found = true;
                        }
                        mysql_free_result(result_set);
                    }
                }
            } else {
                mysql_free_result(result_set);
            }
        }
    }
    
    db_release_connection(conn);
    
    if (!accounts_found) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find account information for transfer");
        return result;
    }
    
    // Start a database transaction to ensure atomicity
    conn = (MYSQL*)db_get_connection();
    if (!conn) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Database connection error");
        return result;
    }
    
    mysql_query(conn, "START TRANSACTION");
    
    // Update sender balance
    float new_sender_balance = sender_balance - amount;
    sprintf(query, "UPDATE %s SET balance = %.2f, lastTransaction = NOW() WHERE accountNumber = '%s'", 
            TABLE_ACCOUNTS, new_sender_balance, sender_account);
    
    if (mysql_query(conn, query) != 0) {
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update sender account balance");
        return result;
    }
    
    // Update receiver balance
    float new_receiver_balance = receiver_balance + amount;
    sprintf(query, "UPDATE %s SET balance = %.2f, lastTransaction = NOW() WHERE accountNumber = '%s'", 
            TABLE_ACCOUNTS, new_receiver_balance, receiver_account);
    
    if (mysql_query(conn, query) != 0) {
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update receiver account balance");
        return result;
    }
    
    // Create transaction record for sender
    sprintf(query, "INSERT INTO %s (cardNumber, accountNumber, amount, timestamp, type, status, remarks) "
            "VALUES (%d, '%s', %.2f, NOW(), 'Transfer', 'Success', 'Transfer to account %s')",
            TABLE_TRANSACTIONS, sender_card, sender_account, amount, receiver_account);
    
    if (mysql_query(conn, query) != 0) {
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to record transaction");
        return result;
    }
    
    // Create transaction record for receiver
    sprintf(query, "INSERT INTO %s (cardNumber, accountNumber, amount, timestamp, type, status, remarks) "
            "VALUES (%d, '%s', %.2f, NOW(), 'Deposit', 'Success', 'Transfer from account %s')",
            TABLE_TRANSACTIONS, receiver_card, receiver_account, amount, sender_account);
    
    if (mysql_query(conn, query) != 0) {
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to record transaction");
        return result;
    }
    
    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0) {
        mysql_query(conn, "ROLLBACK");
        db_release_connection(conn);
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to commit transaction");
        return result;
    }
    
    db_release_connection(conn);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_before = sender_balance;
    result.balance_after = new_sender_balance;
    sprintf(result.message, "Transfer successful. New balance: $%.2f", new_sender_balance);
    
    return result;
}

// Process PIN change
TransactionResult process_pin_change(int card_number, int old_pin, int new_pin) {
    TransactionResult result = {0};
    
    // Validate PIN formats
    if (old_pin < 1000 || old_pin > 9999 || new_pin < 1000 || new_pin > 9999) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "PIN must be a 4-digit number");
        return result;
    }
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Validate current PIN
    if (!validateCard(card_number, old_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Current PIN is incorrect");
        return result;
    }
    
    // Update PIN
    if (!updatePIN(card_number, new_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update PIN");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    strcpy(result.message, "PIN changed successfully");
    
    // Log the transaction
    logTransaction(card_number, "Pin_Change", 0, true);
    
    return result;
}

// Process mini statement
TransactionResult process_mini_statement(int card_number) {
    TransactionResult result = {0};
    
    // Check if card exists
    if (!doesCardExist(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!isCardActive(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = 0.0f;
    if (!fetchBalance(card_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Success - balance will be shown and transactions are handled by UI
    result.status = TRANSACTION_SUCCESS;
    result.balance_before = balance;
    result.balance_after = balance;
    strcpy(result.message, "Mini statement retrieved successfully");
    
    // Log the transaction
    logTransaction(card_number, "Mini_Statement", 0, true);
    
    return result;
}

// Get recent transactions
QueryResult get_recent_transactions(int card_number, int count) {
    QueryResult result = {0};
    
    // Check that card exists
    if (!doesCardExist(card_number)) {
        result.success = 0;
        return result;
    }
    
    // Get account number for this card
    MYSQL *conn = (MYSQL*)db_get_connection();
    if (!conn) {
        result.success = 0;
        return result;
    }
    
    char query[500];
    sprintf(query, "SELECT a.accountNumber FROM %s a JOIN %s c ON a.customerId = c.customerId "
            "JOIN %s cd ON c.customerId = cd.customerId WHERE cd.cardNumber = %d", 
            TABLE_ACCOUNTS, TABLE_CUSTOMERS, TABLE_CARDS, card_number);
    
    char accountNumber[21] = {0};
    
    if (mysql_query(conn, query) != 0) {
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    MYSQL_RES *query_result = mysql_store_result(conn);
    if (!query_result) {
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    MYSQL_ROW row = mysql_fetch_row(query_result);
    if (row && row[0]) {
        strncpy(accountNumber, row[0], sizeof(accountNumber) - 1);
    } else {
        mysql_free_result(query_result);
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    mysql_free_result(query_result);
    
    // Get transactions for this account
    sprintf(query, "SELECT transactionId, cardNumber, type, amount, timestamp, status FROM %s "
            "WHERE accountNumber = '%s' ORDER BY timestamp DESC LIMIT %d", 
            TABLE_TRANSACTIONS, accountNumber, count);
    
    if (mysql_query(conn, query) != 0) {
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    query_result = mysql_store_result(conn);
    if (!query_result) {
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    int num_rows = mysql_num_rows(query_result);
    if (num_rows == 0) {
        mysql_free_result(query_result);
        db_release_connection(conn);
        
        // No transactions, but not an error
        result.success = 1;
        result.count = 0;
        result.data = NULL;
        return result;
    }
    
    // Allocate memory for transactions
    Transaction* transactions = (Transaction*)malloc(num_rows * sizeof(Transaction));
    if (!transactions) {
        mysql_free_result(query_result);
        db_release_connection(conn);
        result.success = 0;
        return result;
    }
    
    int i = 0;
    while ((row = mysql_fetch_row(query_result)) && i < num_rows) {
        transactions[i].id = atoi(row[0]);
        transactions[i].card_number = atoi(row[1]);
        
        // Parse type into enum and string
        if (strcmp(row[2], "Withdrawal") == 0) {
            transactions[i].type = TRANSACTION_WITHDRAWAL;
        } else if (strcmp(row[2], "Deposit") == 0) {
            transactions[i].type = TRANSACTION_DEPOSIT;
        } else if (strcmp(row[2], "Transfer") == 0) {
            transactions[i].type = TRANSACTION_TRANSFER;
        } else if (strcmp(row[2], "Balance") == 0) {
            transactions[i].type = TRANSACTION_BALANCE;
        } else if (strcmp(row[2], "Pin_Change") == 0) {
            transactions[i].type = TRANSACTION_PIN_CHANGE;
        } else if (strcmp(row[2], "Mini_Statement") == 0) {
            transactions[i].type = TRANSACTION_MINI_STATEMENT;
        } else if (strcmp(row[2], "Bill_Payment") == 0) {
            transactions[i].type = TRANSACTION_BILL_PAYMENT;
        } else {
            transactions[i].type = TRANSACTION_BALANCE; // Default
        }
        
        strncpy(transactions[i].transaction_type, row[2], sizeof(transactions[i].transaction_type) - 1);
        transactions[i].transaction_type[sizeof(transactions[i].transaction_type) - 1] = '\0';
        
        transactions[i].amount = row[3] ? atof(row[3]) : 0.0;
        
        // Format timestamp
        strncpy(transactions[i].timestamp, row[4], sizeof(transactions[i].timestamp) - 1);
        transactions[i].timestamp[sizeof(transactions[i].timestamp) - 1] = '\0';
        
        // Status
        strncpy(transactions[i].status, row[5], sizeof(transactions[i].status) - 1);
        transactions[i].status[sizeof(transactions[i].status) - 1] = '\0';
        
        i++;
    }
    
    mysql_free_result(query_result);
    db_release_connection(conn);
    
    result.success = 1;
    result.count = i;
    result.data = transactions;
    
    return result;
}