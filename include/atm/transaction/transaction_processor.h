#ifndef TRANSACTION_PROCESSOR_H
#define TRANSACTION_PROCESSOR_H

#include "transaction_types.h"

// Transaction status codes
typedef enum {
    TRANSACTION_SUCCESS,
    TRANSACTION_FAILED,
    TRANSACTION_PENDING,
    TRANSACTION_CANCELED
} TransactionStatus;

// Transaction result structure 
typedef struct {
    TransactionStatus status;  // Success or failure
    double amount_processed;   // Amount actually processed
    double balance_before;     // Balance before transaction
    double balance_after;      // Balance after transaction
    char message[200];         // Result message
} TransactionResult;

// Transaction processing functions
TransactionResult process_balance_inquiry(int card_number);
TransactionResult process_withdrawal(int card_number, float amount);
TransactionResult process_deposit(int card_number, float amount);
TransactionResult process_transfer(int sender_card, int receiver_card, float amount);
TransactionResult process_pin_change(int card_number, int old_pin, int new_pin);
TransactionResult process_mini_statement(int card_number);
TransactionResult process_bill_payment(int card_number, const char* bill_type, const char* bill_reference, float amount);

// Query functions
QueryResult get_recent_transactions(int card_number, int count);

#endif // TRANSACTION_PROCESSOR_H