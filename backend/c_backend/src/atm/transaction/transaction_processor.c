#include "../../../../../include/atm/transaction/transaction_processor.h"
#include <stdbool.h>

// Function prototype for function not declared in headers
bool cbs_update_daily_withdrawal(const char* card_number, double amount);

// Temporarily rename the transaction types from core_banking_interface.h to avoid conflicts
#define TRANSACTION_WITHDRAWAL CBS_TRANSACTION_WITHDRAWAL
#define TRANSACTION_DEPOSIT CBS_TRANSACTION_DEPOSIT
#define TRANSACTION_TRANSFER CBS_TRANSACTION_TRANSFER
#define TRANSACTION_PAYMENT CBS_TRANSACTION_PAYMENT
#define TRANSACTION_BALANCE_INQUIRY CBS_TRANSACTION_BALANCE_INQUIRY
#define TRANSACTION_MINI_STATEMENT CBS_TRANSACTION_MINI_STATEMENT
#define TRANSACTION_PIN_CHANGE CBS_TRANSACTION_PIN_CHANGE
#define TRANSACTION_INTEREST_CREDIT CBS_TRANSACTION_INTEREST_CREDIT
#define TRANSACTION_FEE_DEBIT CBS_TRANSACTION_FEE_DEBIT
#define TRANSACTION_REVERSAL CBS_TRANSACTION_REVERSAL

#include "../../../../../include/common/database/card_account_management.h"
#include "../../../../../include/common/database/account_management.h"
#include "../../../../../include/common/database/core_banking_interface.h"

// Undefine the renamed types so we can use our local ones
#undef TRANSACTION_WITHDRAWAL
#undef TRANSACTION_DEPOSIT
#undef TRANSACTION_TRANSFER
#undef TRANSACTION_PAYMENT
#undef TRANSACTION_BALANCE_INQUIRY
#undef TRANSACTION_MINI_STATEMENT
#undef TRANSACTION_PIN_CHANGE
#undef TRANSACTION_INTEREST_CREDIT
#undef TRANSACTION_FEE_DEBIT
#undef TRANSACTION_REVERSAL

#include "../../../../../include/common/database/db_config.h"
#include "../../../../../include/common/utils/logger.h"
#include "../../../../../include/common/transaction/bill_payment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Process balance inquiry using core banking system
TransactionResult process_balance_inquiry(int card_number) {
    TransactionResult result = {0};
    
    // Check if card exists using new CBS function
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active using new CBS function
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get the balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = 0;
    result.balance_before = (float)balance;
    result.balance_after = (float)balance;
    sprintf(result.message, "Current balance: $%.2f", (float)balance);
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        // Log the transaction using core banking system
        char transaction_id[37]; // UUID format
        cbs_process_transaction(account_number, CBS_TRANSACTION_BALANCE_INQUIRY, "ATM", 0.0, transaction_id);
    } else {
        writeErrorLog("Failed to get account number for card %d", card_number);
    }
    
    return result;
}

// Process withdrawal using core banking system
TransactionResult process_withdrawal(int card_number, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid withdrawal amount");
        return result;
    }
    
    // Check if card exists
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Convert card number to string
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find account linked to card");
        return result;
    }
    
    // Get current balance
    double balance = 0.0;
    if (!cbs_get_balance(account_number, &balance)) {
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
    
    // Check withdrawal limits
    double remaining_limit = 0.0;
    if (!cbs_check_withdrawal_limit(card_number_str, amount, "ATM", &remaining_limit)) {
        result.status = TRANSACTION_FAILED;
        sprintf(result.message, "Withdrawal limit exceeded. Remaining limit: $%.2f", remaining_limit);
        return result;
    }
    
    // Calculate new balance
    double new_balance = balance - amount;
    
    // Update the balance
    if (!cbs_update_balance(account_number, new_balance, "WITHDRAWAL")) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to process withdrawal transaction");
        return result;
    }
    
    // Process transaction record
    char transaction_id[37]; // UUID format
    cbs_process_transaction(account_number, CBS_TRANSACTION_WITHDRAWAL, "ATM", amount, transaction_id);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = (float)amount;
    result.balance_before = (float)balance;
    result.balance_after = (float)new_balance;
    sprintf(result.message, "Withdrawal successful. New balance: $%.2f", (float)new_balance);
    
    // Update daily withdrawal tracking
    cbs_update_daily_withdrawal(card_number_str, amount);
    
    return result;
}

// Process deposit using core banking system
TransactionResult process_deposit(int card_number, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid deposit amount");
        return result;
    }
    
    // Check if card exists
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Convert card number to string
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find account linked to card");
        return result;
    }
    
    // Get current balance
    double balance = 0.0;
    if (!cbs_get_balance(account_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Calculate new balance
    double new_balance = balance + amount;
    
    // Update the balance
    if (!cbs_update_balance(account_number, new_balance, "DEPOSIT")) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to process deposit transaction");
        return result;
    }
    
    // Process transaction record
    char transaction_id[37]; // UUID format
    cbs_process_transaction(account_number, CBS_TRANSACTION_DEPOSIT, "ATM", amount, transaction_id);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = (float)amount;
    result.balance_before = (float)balance;
    result.balance_after = (float)new_balance;
    sprintf(result.message, "Deposit successful. New balance: $%.2f", (float)new_balance);
    
    return result;
}

// Process transfer using core banking system
TransactionResult process_transfer(int sender_card, int receiver_card, float amount) {
    TransactionResult result = {0};
    
    // Validate amount
    if (amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid transfer amount");
        return result;
    }
    
    // Check if cards are valid
    if (!cbs_card_exists(sender_card) || !cbs_card_exists(receiver_card)) {
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
    if (!cbs_is_card_active(sender_card) || !cbs_is_card_active(receiver_card)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "One of the cards is inactive or blocked");
        return result;
    }
    
    // Convert card numbers to string
    char sender_card_str[20], receiver_card_str[20];
    snprintf(sender_card_str, sizeof(sender_card_str), "%d", sender_card);
    snprintf(receiver_card_str, sizeof(receiver_card_str), "%d", receiver_card);
    
    // Get account numbers linked to these cards
    char sender_account[25] = {0}, receiver_account[25] = {0};
    
    if (!cbs_get_account_by_card(sender_card_str, sender_account, sizeof(sender_account))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find sender account");
        return result;
    }
    
    if (!cbs_get_account_by_card(receiver_card_str, receiver_account, sizeof(receiver_account))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find receiver account");
        return result;
    }
    
    // Get sender balance
    double sender_balance = 0.0;
    if (!cbs_get_balance(sender_account, &sender_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve sender account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (sender_balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds for transfer");
        return result;
    }
    
    // Get receiver balance
    double receiver_balance = 0.0;
    if (!cbs_get_balance(receiver_account, &receiver_balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve receiver account balance");
        return result;
    }
    
    // Perform the transfer
    char transaction_id[37];
    if (!cbs_transfer_funds(sender_account, receiver_account, amount, "INTERNAL", transaction_id)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to process transfer");
        return result;
    }
    
    // Get updated balances
    double updated_sender_balance = 0.0;
    cbs_get_balance(sender_account, &updated_sender_balance);
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = (float)amount;
    result.balance_before = (float)sender_balance;
    result.balance_after = (float)updated_sender_balance;
    sprintf(result.message, "Transfer successful. New balance: $%.2f", (float)updated_sender_balance);
    
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
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Validate current PIN
    if (!cbs_validate_card(card_number, old_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Current PIN is incorrect");
        return result;
    }
    
    // Update PIN
    if (!cbs_update_pin(card_number, new_pin)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to update PIN");
        return result;
    }
    
    // Get account number linked to this card for transaction logging
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    char account_number[25] = {0};
    if (cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        // Log the PIN change transaction
        char transaction_id[37];
        cbs_process_transaction(account_number, CBS_TRANSACTION_PIN_CHANGE, "ATM", 0.0, transaction_id);
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    strcpy(result.message, "PIN has been successfully changed");
    
    return result;
}

// Process mini statement using core banking system
TransactionResult process_mini_statement(int card_number) {
    TransactionResult result = {0};
    
    // Check if card exists
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Convert card number to string
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find account linked to card");
        return result;
    }
    
    // Get current balance
    double balance = 0.0;
    if (!cbs_get_balance(account_number, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Log transaction in the core banking system
    char transaction_id[37]; // UUID format
    cbs_process_transaction(account_number, CBS_TRANSACTION_MINI_STATEMENT, "ATM", 0.0, transaction_id);
    
    // Success - balance will be shown and transactions are handled by UI
    result.status = TRANSACTION_SUCCESS;
    result.balance_before = (float)balance;
    result.balance_after = (float)balance;
    strcpy(result.message, "Mini statement retrieved successfully");
    
    return result;
}

// Process bill payment using core banking system
TransactionResult process_bill_payment(int card_number, const char* bill_type, const char* bill_reference, float amount) {
    TransactionResult result = {0};
    
    // Check parameters
    if (!bill_type || !bill_reference || amount <= 0) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid bill payment parameters");
        return result;
    }
    
    // Check if card exists using CBS function
    if (!cbs_card_exists(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Invalid card number");
        return result;
    }
    
    // Check if card is active using CBS function
    if (!cbs_is_card_active(card_number)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Card is inactive or blocked");
        return result;
    }
    
    // Get the account balance
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    double balance = 0.0;
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not retrieve account balance");
        return result;
    }
    
    // Check if sufficient balance
    if (balance < amount) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Insufficient funds for bill payment");
        writeInfoLog("Bill payment failed due to insufficient funds: Card %d, Amount %.2f, Balance %.2f", 
                     card_number, amount, balance);
        return result;
    }
    
    // Store balance before transaction
    result.balance_before = balance;
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Could not find account linked to card");
        return result;
    }
    
    // Process the bill payment by deducting amount from account
    // Calculate new balance
    double new_balance = balance - amount;
    
    // Update the balance
    if (!cbs_update_balance(account_number, new_balance, "BILL_PAYMENT")) {
        result.status = TRANSACTION_FAILED;
        strcpy(result.message, "Failed to process bill payment transaction");
        return result;
    }
      // Create transaction record in core banking
    char transaction_id[37] = {0};
    cbs_process_transaction(account_number, CBS_TRANSACTION_PAYMENT, "ATM", amount, transaction_id);
    
    // Also record in bill payment system
    // We're directly calling cbs_process_transaction, no need to try cbs_process_bill_payment
    // which might not be linked properly
    
    // Get updated balance
    if (!cbs_get_balance_by_card(card_number_str, &balance)) {
        // Payment succeeded but couldn't fetch new balance
        result.status = TRANSACTION_SUCCESS;
        result.amount_processed = amount;
        result.balance_after = result.balance_before - amount; // Estimate new balance
        sprintf(result.message, "Bill payment successful. Transaction ID: %s", transaction_id);
        writeInfoLog("Bill payment successful: Card %d, Bill Type %s, Ref %s, Amount %.2f, Transaction ID %s", 
                    card_number, bill_type, bill_reference, amount, transaction_id);
        return result;
    }
    
    // Success
    result.status = TRANSACTION_SUCCESS;
    result.amount_processed = amount;
    result.balance_after = balance;
    sprintf(result.message, "Bill payment successful. Transaction ID: %s", transaction_id);
    
    writeInfoLog("Bill payment successful: Card %d, Bill Type %s, Ref %s, Amount %.2f, Transaction ID %s", 
                card_number, bill_type, bill_reference, amount, transaction_id);
                
    return result;
}

// Get recent transactions using core banking system
QueryResult get_recent_transactions(int card_number, int count) {
    QueryResult result = {0};
    
    // Check that card exists
    if (!cbs_card_exists(card_number)) {
        result.success = 0;
        return result;
    }
    
    // Convert card number to string
    char card_number_str[20];
    snprintf(card_number_str, sizeof(card_number_str), "%d", card_number);
    
    // Get account number linked to this card
    char account_number[25] = {0};
    if (!cbs_get_account_by_card(card_number_str, account_number, sizeof(account_number))) {
        result.success = 0;
        return result;
    }
    
    // Allocate memory for transactions - we'll get at most 'count' transactions
    Transaction* transactions = (Transaction*)malloc(count * sizeof(Transaction));
    if (!transactions) {
        writeErrorLog("Memory allocation failed in get_recent_transactions");
        result.success = 0;
        return result;
    }
    
    // Get mini statement from core banking system
    TransactionRecord* records = (TransactionRecord*)malloc(count * sizeof(TransactionRecord));
    if (!records) {
        free(transactions);
        writeErrorLog("Memory allocation failed for transaction records");
        result.success = 0;
        return result;
    }
    
    int actual_count = 0;
    if (!cbs_get_mini_statement(account_number, records, &actual_count, count)) {
        free(transactions);
        free(records);
        result.success = 0;
        return result;
    }
    
    if (actual_count == 0) {
        // No transactions, but not an error
        free(transactions);
        free(records);
        result.success = 1;
        result.count = 0;
        result.data = NULL;
        return result;
    }
    
    // Convert CBS transaction records to our transaction format
    for (int i = 0; i < actual_count; i++) {
        transactions[i].card_number = card_number; // Use the original card number
        
        // Parse type into enum
        if (strcmp(records[i].transaction_type, "WITHDRAWAL") == 0) {
            transactions[i].type = TRANSACTION_WITHDRAWAL;
            strncpy(transactions[i].transaction_type, "Withdrawal", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "DEPOSIT") == 0) {
            transactions[i].type = TRANSACTION_DEPOSIT;
            strncpy(transactions[i].transaction_type, "Deposit", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "TRANSFER") == 0) {
            transactions[i].type = TRANSACTION_TRANSFER;
            strncpy(transactions[i].transaction_type, "Transfer", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "BALANCE_INQUIRY") == 0) {
            transactions[i].type = TRANSACTION_BALANCE;
            strncpy(transactions[i].transaction_type, "Balance", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "PIN_CHANGE") == 0) {
            transactions[i].type = TRANSACTION_PIN_CHANGE;
            strncpy(transactions[i].transaction_type, "Pin_Change", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "MINI_STATEMENT") == 0) {
            transactions[i].type = TRANSACTION_MINI_STATEMENT;
            strncpy(transactions[i].transaction_type, "Mini_Statement", sizeof(transactions[i].transaction_type) - 1);
        } else if (strcmp(records[i].transaction_type, "PAYMENT") == 0) {
            transactions[i].type = TRANSACTION_BILL_PAYMENT;
            strncpy(transactions[i].transaction_type, "Bill_Payment", sizeof(transactions[i].transaction_type) - 1);
        } else {
            transactions[i].type = TRANSACTION_BALANCE; // Default for unknown types
            strncpy(transactions[i].transaction_type, records[i].transaction_type, sizeof(transactions[i].transaction_type) - 1);
        }
        
        transactions[i].transaction_type[sizeof(transactions[i].transaction_type) - 1] = '\0';
        
        // Set amount and balance
        transactions[i].amount = (float)records[i].amount;
        transactions[i].balance = (float)records[i].balance;
        
        // Format timestamp
        strncpy(transactions[i].timestamp, records[i].date, sizeof(transactions[i].timestamp) - 1);
        transactions[i].timestamp[sizeof(transactions[i].timestamp) - 1] = '\0';
        
        // Copy transaction ID 
        strncpy(transactions[i].transaction_id, records[i].transaction_id, sizeof(transactions[i].transaction_id) - 1);
        transactions[i].transaction_id[sizeof(transactions[i].transaction_id) - 1] = '\0';
        
        // Set status based on CBS status
        transactions[i].status = (strcmp(records[i].status, "SUCCESS") == 0) ? 1 : 0;
    }
    
    free(records); // Release the CBS records after conversion
    
    // Set the result
    result.success = 1;
    result.count = actual_count;
    result.data = transactions;
    
    return result;
}