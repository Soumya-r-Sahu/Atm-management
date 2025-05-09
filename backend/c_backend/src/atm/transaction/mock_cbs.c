// Mock implementation of core banking functions for testing
#include "../../../../../include/common/database/core_banking_interface.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stddef.h> // For size_t

// Internal helper functions
static void generate_transaction_id(char* transaction_id_out) {
    // Generate a simple transaction ID based on timestamp
    sprintf(transaction_id_out, "MOCK-TRANS-%ld", (long)time(NULL));
}

// Card verification
bool cbs_card_exists(int card_number) {
    printf("MOCK: Checking if card %d exists\n", card_number);
    return true;  // Always return true for testing
}

// Card activation check
bool cbs_is_card_active(int card_number) {
    printf("MOCK: Checking if card %d is active\n", card_number);
    return true;  // Always active for testing
}

// Account balance - matching the interface from core_banking_interface.h
bool cbs_get_account_balance(const char* account_number, double* balance_out) {
    printf("MOCK: Getting balance for account %s\n", account_number);
    *balance_out = 5000.00;  // Return a mock balance
    return true;
}

// Transaction processing - matching the function call in transaction_processor.c
bool cbs_process_transaction(const char* account_number, 
                            TRANSACTION_TYPE type, 
                            const char* channel, 
                            double amount, 
                            char* transaction_id_out) {
    printf("MOCK: Processing transaction type %d for account %s, amount: %.2f via %s\n", 
           type, account_number, amount, channel);
    
    generate_transaction_id(transaction_id_out);
    return true;  // Always succeed for testing
}

// Transfer funds function
bool cbs_transfer_funds(const char* source_account, 
                       const char* destination_account, 
                       double amount, 
                       const char* transfer_type,
                       char* transaction_id_out) {
    printf("MOCK: Transferring %.2f from %s to %s via %s\n", 
           amount, source_account, destination_account, transfer_type);
    
    generate_transaction_id(transaction_id_out);
    return true;
}

// Mini statement function
bool cbs_get_mini_statement(const char* account_number, 
                           TransactionRecord* records, 
                           int* count, 
                           int max_records) {
    printf("MOCK: Getting mini statement for account %s (max %d records)\n", 
           account_number, max_records);
    
    // Generate a few mock transactions
    *count = max_records > 3 ? 3 : max_records;
    
    for(int i = 0; i < *count; i++) {
        sprintf(records[i].transaction_id, "MOCK-TRANS-%d", i+1);
        sprintf(records[i].transaction_type, i % 2 == 0 ? "DEPOSIT" : "WITHDRAWAL");
        records[i].amount = (i+1) * 100.0;
        records[i].balance = 5000.0 - (i * 100.0);
        strcpy(records[i].date, "2025-05-09 10:00:00");
        strcpy(records[i].status, "SUCCESS");
    }
    
    return true;
}

// Daily withdrawal limits
bool cbs_check_withdrawal_limit(const char* card_number, 
                               double amount, 
                               const char* channel, 
                               double* remaining_limit) {
    printf("MOCK: Checking withdrawal limit for card %s, amount: %.2f via %s\n", 
           card_number, amount, channel);
    
    *remaining_limit = 10000.0 - amount;  // Mock a daily limit of 10,000
    return amount <= 10000.0;  // Allow if within limit
}

// Daily withdrawal update (for our internal interface)
bool cbs_update_daily_withdrawal(const char* card_number, double amount) {
    printf("MOCK: Updating daily withdrawal for card %s, amount: %.2f\n", 
           card_number, amount);
    return true;  // Always succeed for testing
}

// Get account by card function
bool cbs_get_account_by_card(const char* card_number, 
                           char* account_number_out, 
                           size_t account_number_size) {
    printf("MOCK: Getting account for card %s\n", card_number);
    snprintf(account_number_out, account_number_size, "ACC-%s", card_number);
    return true;
}

// Get balance by card - matches function call in transaction_processor.c
bool cbs_get_balance_by_card(const char* card_number, double* balance_out) {
    printf("MOCK: Getting balance for card %s\n", card_number);
    *balance_out = 5000.00;
    return true;
}

// Get card details
bool cbs_get_card_details(int card_number, 
                        char* holder_name_out, size_t holder_name_size,
                        char* account_number_out, size_t account_number_size,
                        char* expiry_date_out, size_t expiry_date_size,
                        int* is_active_out,
                        double* daily_limit_out,
                        char* card_type_out, size_t card_type_size) {
    printf("MOCK: Getting card details for %d\n", card_number);
    
    snprintf(holder_name_out, holder_name_size, "John Doe");
    snprintf(account_number_out, account_number_size, "ACC-%d", card_number);
    snprintf(expiry_date_out, expiry_date_size, "12/28");
    *is_active_out = 1;  // using int instead of bool
    *daily_limit_out = 10000.00;
    snprintf(card_type_out, card_type_size, "VISA GOLD");
    
    return true;
}

// Get transaction history
bool cbs_get_transaction_history(const char* account_number, 
                                TransactionRecord* records, 
                                int* count, 
                                int max_records) {
    // Just reuse the mini statement function for simplicity
    return cbs_get_mini_statement(account_number, records, count, max_records);
}

// Update account balance
bool cbs_update_balance(const char* account_number, double new_balance, const char* transaction_type) {
    printf("MOCK: Updating balance for account %s to %.2f (type: %s)\n", 
           account_number, new_balance, transaction_type);
    return true;  // Always succeed for testing
}

// Get account balance - directly from account number
bool cbs_get_balance(const char* account_number, double* balance_out) {
    printf("MOCK: Getting balance for account %s\n", account_number);
    *balance_out = 5000.00;
    return true;
}

// Update account balance
bool cbs_update_balance(const char* account_number, double new_balance, const char* transaction_type) {
    printf("MOCK: Updating balance for account %s to $%.2f (type: %s)\n", 
           account_number, new_balance, transaction_type);
    return true;
}

// Validate card PIN
bool cbs_validate_card(const char* card_number, int pin) {
    printf("MOCK: Validating PIN for card %s\n", card_number);
    return pin != 0000; // Consider 0000 as invalid for testing purposes
}

// Update card PIN
bool cbs_update_pin(const char* card_number, int new_pin) {
    printf("MOCK: Updating PIN for card %s to %04d\n", card_number, new_pin);
    return true;
}
