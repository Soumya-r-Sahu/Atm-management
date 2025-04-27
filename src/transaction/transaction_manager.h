#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "transaction_types.h"
#include <unistd.h>  // For getpid()

// Transaction result structure
typedef struct {
    int success;           // 1 for success, 0 for failure
    float oldBalance;      // Balance before transaction
    float newBalance;      // Balance after transaction
    char message[200];     // Result message or error
} TransactionResult;

// Balance check operation
TransactionResult checkAccountBalance(int cardNumber, const char* username);

// Deposit operation
TransactionResult performDeposit(int cardNumber, float amount, const char* username);

// Withdrawal operation
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username);

// Mini-statement operation
TransactionResult getMiniStatement(int cardNumber, const char* username);

// Money transfer operation (with transaction atomicity)
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username);

// Alias for backward compatibility
TransactionResult performFundTransfer(int cardNumber, int targetCardNumber, float amount, const char* username);

// Transaction atomicity helpers
int lockTransactionFiles();
int unlockTransactionFiles();
int backupAccountFiles();
int restoreAccountFiles();

// Transaction logging
void writeTransactionDetails(const char* username, const char* type, const char* details);
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber);

#endif // TRANSACTION_MANAGER_H