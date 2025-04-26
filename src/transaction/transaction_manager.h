#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#define PHONE_NUMBER_LENGTH 10

// Transaction type enumeration
typedef enum {
    TRANSACTION_BALANCE_CHECK,
    TRANSACTION_DEPOSIT,
    TRANSACTION_WITHDRAWAL,
    TRANSACTION_PIN_CHANGE,
    TRANSACTION_MINI_STATEMENT,
    TRANSACTION_MONEY_TRANSFER
} TransactionType;

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

// Function to log transaction details
void logTransaction(int cardNumber, TransactionType type, float amount, int success);

// Function to generate a transaction receipt
void generateReceipt(int cardNumber, TransactionType type, float amount, float balance, const char* phoneNumber);

#endif // TRANSACTION_MANAGER_H