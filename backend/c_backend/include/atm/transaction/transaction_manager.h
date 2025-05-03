#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "transaction_types.h"  // Local include since it's in the same directory
#include <unistd.h>  // For getpid()

// Transaction types
typedef enum {
    TRANSACTION_BALANCE_CHECK,
    TRANSACTION_DEPOSIT,
    TRANSACTION_WITHDRAWAL,
    TRANSACTION_MONEY_TRANSFER,
    TRANSACTION_MINI_STATEMENT,
    TRANSACTION_PIN_CHANGE,
    TRANSACTION_BILL_PAYMENT    // Added new transaction type for bill payments
} TransactionType;

// Transaction result structure
typedef struct {
    int success;          // 1 = success, 0 = failure
    char message[1024];   // Message describing the result
    float oldBalance;     // Balance before transaction
    float newBalance;     // Balance after transaction
} TransactionResult;

// Basic ATM transaction functions
TransactionResult checkAccountBalance(int cardNumber, const char* username);
TransactionResult performDeposit(int cardNumber, float amount, const char* username);
TransactionResult performWithdrawal(int cardNumber, float amount, const char* username);
TransactionResult getMiniStatement(int cardNumber, const char* username);
TransactionResult performMoneyTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username);
TransactionResult performFundTransfer(int senderCardNumber, int receiverCardNumber, float amount, const char* username);
TransactionResult performBillPayment(int cardNumber, float amount, const char* billerName, const char* accountId, const char* username); // Updated signature to match implementation

// Virtual ATM transaction functions
TransactionResult checkAccountBalanceByCardNumber(const char* cardNumber, const char* username);
TransactionResult performVirtualDeposit(const char* cardNumber, int cvv, const char* expiryDate, float amount, const char* username);
TransactionResult performVirtualWithdrawal(const char* cardNumber, int cvv, const char* expiryDate, float amount, const char* username);
TransactionResult getVirtualMiniStatement(const char* cardNumber, int cvv, const char* expiryDate, const char* username);
TransactionResult performVirtualMoneyTransfer(const char* senderCardNumber, int cvv, const char* expiryDate, 
                                             const char* receiverCardNumber, float amount, const char* username);
TransactionResult performVirtualBillPayment(const char* cardNumber, int cvv, const char* expiryDate, 
                                           float amount, const char* billerName, const char* accountId, 
                                           const char* username); // Updated signature to match implementation

// Receipt functions
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber);
void generateVirtualReceipt(const char* cardNumber, const char* customerName, TransactionType type, float amount, float balance);
void generateAccountCreationReceipt(const char* customerName, const char* accountNumber, 
                                   const char* accountType, const char* ifscCode, 
                                   const char* cardNumber, const char* expiryDate, int cvv);

// Transaction atomicity functions
int lockTransactionFiles();
int unlockTransactionFiles();
int backupAccountFiles();
int restoreAccountFiles();

#endif // TRANSACTION_MANAGER_H