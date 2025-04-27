#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <stdbool.h>  // Added for bool type
#include "transaction_types.h"  // Include shared TransactionType definition

#define PHONE_NUMBER_LENGTH 10

// Transaction result structure
typedef struct {
    int success;
    char message[200];
    float newBalance;
    float oldBalance;
} TransactionResult;

// Function declarations
TransactionResult checkAccountBalance(int cardNumber, const char* username);
TransactionResult performDeposit(int cardNumber, float amount, const char* username);
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username);
TransactionResult getMiniStatement(int cardNumber, const char* username);
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username);
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username);

// Function to log transaction details
void logTransaction(int cardNumber, TransactionType type, float amount, bool success);

// Helper function to write transaction log entries (renamed to avoid conflict)
void writeTransactionDetails(const char* username, const char* transactionType, const char* details);

// Function to generate a transaction receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber);

#endif // TRANSACTION_MANAGER_H