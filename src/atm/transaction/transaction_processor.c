#include "atm/transaction/transaction_processor.h"
// Remove reference to transaction_manager.h since it has conflicting types
// #include "atm/transaction/transaction_manager.h"
#include "common/database/database.h"
#include "common/database/database_compat.h"
#include "common/utils/logger.h"
#include "common/utils/hash_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Add missing function prototypes
bool is_card_active(int card_number);
float fetch_balance(int card_number);
bool update_balance(int card_number, float new_balance);
void log_transaction(int card_number, TransactionType type, float amount, bool success);
void log_withdrawal(int card_number, float amount);
bool has_exceeded_daily_limit(int card_number, float amount);
bool validate_card_pin(int card_number, int pin);
bool update_pin(int card_number, int new_pin);
bool is_valid_pin(int pin);
bool does_card_exist(int card_number);

// Process balance inquiry
TransactionResult process_balance_inquiry(int card_number) {
    TransactionResult result = {0};
    
    // Get the balance
    float balance = fetch_balance(card_number);
    if (balance < 0) {
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
    log_transaction(card_number, TRANSACTION_BALANCE, 0, true);
    
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
    
    // Check if card is active
    if (!is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = fetch_balance(card_number);
    if (balance < 0) {
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
    
    // Check daily withdrawal limit
    if (has_exceeded_daily_limit(card_number, amount)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Daily withdrawal limit exceeded");
        return result;
    }
    
    // Update balance
    float new_balance = balance - amount;
    if (!update_balance(card_number, new_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update account balance");
        return result;
    }
    
    // Log the withdrawal
    log_withdrawal(card_number, amount);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_before = balance;
    result.balance_after = new_balance;
    sprintf(result.message, "Withdrawal successful. New balance: $%.2f", new_balance);
    
    // Log the transaction
    log_transaction(card_number, TRANSACTION_WITHDRAWAL, amount, true);
    
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
    
    // Check if card is active
    if (!is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = fetch_balance(card_number);
    if (balance < 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Update balance
    float new_balance = balance + amount;
    if (!update_balance(card_number, new_balance)) {
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
    log_transaction(card_number, TRANSACTION_DEPOSIT, amount, true);
    
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
    if (!does_card_exist(sender_card) || !does_card_exist(receiver_card)) {
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
    if (!is_card_active(sender_card) || !is_card_active(receiver_card)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "One of the cards is inactive or blocked");
        return result;
    }
    
    // Get sender balance
    float sender_balance = fetch_balance(sender_card);
    if (sender_balance < 0) {
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
    float receiver_balance = fetch_balance(receiver_card);
    if (receiver_balance < 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve receiver account balance");
        return result;
    }
    
    // Update sender balance
    float new_sender_balance = sender_balance - amount;
    if (!update_balance(sender_card, new_sender_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update sender account balance");
        return result;
    }
    
    // Update receiver balance
    float new_receiver_balance = receiver_balance + amount;
    if (!update_balance(receiver_card, new_receiver_balance)) {
        // Undo sender balance change
        update_balance(sender_card, sender_balance);
        
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update receiver account balance");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_before = sender_balance;
    result.balance_after = new_sender_balance;
    sprintf(result.message, "Transfer successful. New balance: $%.2f", new_sender_balance);
    
    // Log the transactions
    log_transaction(sender_card, TRANSACTION_TRANSFER, amount, true);
    log_transaction(receiver_card, TRANSACTION_DEPOSIT, amount, true);
    
    return result;
}

// Process PIN change
TransactionResult process_pin_change(int card_number, int old_pin, int new_pin) {
    TransactionResult result = {0};
    
    // Validate PIN formats
    if (!is_valid_pin(old_pin) || !is_valid_pin(new_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid PIN format");
        return result;
    }
    
    // Validate current PIN
    if (!validate_card_pin(card_number, old_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Current PIN is incorrect");
        return result;
    }
    
    // Update PIN
    if (!update_pin(card_number, new_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update PIN");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    strcpy(result.message, "PIN changed successfully");
    
    // Log the transaction
    log_transaction(card_number, TRANSACTION_PIN_CHANGE, 0, true);
    
    return result;
}

// Process mini statement
TransactionResult process_mini_statement(int card_number) {
    TransactionResult result = {0};
    
    // Check if card is active
    if (!is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get current balance
    float balance = fetch_balance(card_number);
    if (balance < 0) {
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
    log_transaction(card_number, TRANSACTION_MINI_STATEMENT, 0, true);
    
    return result;
}

// Get recent transactions
QueryResult get_recent_transactions(int card_number, int count) {
    QueryResult result = {0};
    
    // Allocate memory for transactions
    Transaction* transactions = (Transaction*)malloc(count * sizeof(Transaction));
    if (!transactions) {
        result.success = 0;
        return result;
    }
    
    // In a real system, this would query a database
    // Here we're just creating sample data
    time_t now = time(NULL);
    
    // Get current balance for reference
    float balance = fetch_balance(card_number);
    
    for (int i = 0; i < count; i++) {
        transactions[i].id = i + 1;
        transactions[i].card_number = card_number;
        
        // Create descending dates
        struct tm tm_time = *localtime(&now);
        tm_time.tm_mday -= i;
        mktime(&tm_time);
        strftime(transactions[i].timestamp, sizeof(transactions[i].timestamp), 
                 "%Y-%m-%d", &tm_time);
        
        // Alternate transaction types
        switch (i % 5) {
            case 0:
                transactions[i].type = TRANSACTION_WITHDRAWAL;
                strcpy(transactions[i].transaction_type, "Withdrawal");
                transactions[i].amount = 50.0 * (i + 1);
                break;
            case 1:
                transactions[i].type = TRANSACTION_DEPOSIT;
                strcpy(transactions[i].transaction_type, "Deposit");
                transactions[i].amount = 100.0 * (i + 1);
                break;
            case 2:
                transactions[i].type = TRANSACTION_TRANSFER;
                strcpy(transactions[i].transaction_type, "Transfer");
                transactions[i].amount = 75.0 * (i + 1);
                break;
            case 3:
                transactions[i].type = TRANSACTION_BALANCE;
                strcpy(transactions[i].transaction_type, "Balance");
                transactions[i].amount = 0.0;
                break;
            case 4:
                transactions[i].type = TRANSACTION_PIN_CHANGE;
                strcpy(transactions[i].transaction_type, "PIN Change");
                transactions[i].amount = 0.0;
                break;
        }
        
        strcpy(transactions[i].status, "Success");
    }
    
    result.success = 1;
    result.count = count;
    result.data = transactions;
    
    return result;
}