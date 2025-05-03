#include "atm/transaction/transaction_processor.h"
#include "common/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Stub implementations for missing functions referenced in transaction_processor.c

// Check if a card is active
bool is_card_active(int card_number) {
    // Stub implementation: assume card is active for testing
    return true;
}

// Fetch account balance for a card
float fetch_balance(int card_number) {
    // Stub implementation: return a default balance for testing
    return 1000.0f;
}

// Update account balance for a card
bool update_balance(int card_number, float new_balance) {
    // Stub implementation: assume update is successful for testing
    writeInfoLog("Balance updated successfully");
    return true;
}

// Log a withdrawal transaction
void log_withdrawal(int card_number, float amount) {
    // Stub implementation: just log the withdrawal
    char message[100];
    sprintf(message, "Withdrawal of %.2f from card %d", amount, card_number);
    writeInfoLog(message);
}

// Log a generic transaction
void log_transaction(int card_number, TransactionType type, float amount, bool success) {
    // Stub implementation: log the transaction details
    const char* type_str;
    
    switch (type) {
        case TRANSACTION_BALANCE:
            type_str = "Balance Inquiry";
            break;
        case TRANSACTION_WITHDRAWAL:
            type_str = "Withdrawal";
            break;
        case TRANSACTION_DEPOSIT:
            type_str = "Deposit";
            break;
        case TRANSACTION_TRANSFER:
            type_str = "Transfer";
            break;
        case TRANSACTION_PIN_CHANGE:
            type_str = "PIN Change";
            break;
        case TRANSACTION_MINI_STATEMENT:
            type_str = "Mini Statement";
            break;
        case TRANSACTION_CARD_REQUEST:
            type_str = "Card Request";
            break;
        default:
            type_str = "Unknown";
    }
    
    char message[150];
    sprintf(message, "%s transaction for card %d, amount: %.2f, status: %s", 
            type_str, card_number, amount, success ? "Success" : "Failed");
    writeInfoLog(message);
}

// Check if a withdrawal would exceed the daily limit
bool has_exceeded_daily_limit(int card_number, float amount) {
    // Stub implementation: assume no limit exceeded for testing
    return false;
}

// Check if a PIN is a valid format (e.g., 4 digits)
bool is_valid_pin(int pin) {
    // Stub implementation: check if PIN is a 4-digit number
    return (pin >= 1000 && pin <= 9999);
}

// Validate card PIN against database
bool validate_card_pin(int card_number, int pin) {
    // Stub implementation: assume PIN is valid for testing
    return true;
}

// Update PIN in the database
bool update_pin(int card_number, int new_pin) {
    // Stub implementation: assume update is successful for testing
    writeInfoLog("PIN updated successfully");
    return true;
}

// Check if a card exists in the database
bool does_card_exist(int card_number) {
    // Stub implementation: assume card exists for testing
    return true;
}