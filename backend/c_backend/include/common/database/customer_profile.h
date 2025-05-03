#ifndef CUSTOMER_PROFILE_H
#define CUSTOMER_PROFILE_H

#include <stdbool.h>
#include <time.h>

// Customer status enumeration
typedef enum {
    CUSTOMER_ACTIVE,
    CUSTOMER_INACTIVE,
    CUSTOMER_SUSPENDED
} CustomerStatus;

// KYC status enumeration
typedef enum {
    KYC_PENDING,
    KYC_COMPLETED
} KYCStatus;

// Card status enumeration
typedef enum {
    CARD_ACTIVE,
    CARD_BLOCKED,
    CARD_EXPIRED
} CardStatus;

// Account status enumeration
typedef enum {
    ACCOUNT_ACTIVE,
    ACCOUNT_INACTIVE,
    ACCOUNT_CLOSED
} AccountStatus;

// Account type enumeration
typedef enum {
    ACCOUNT_SAVINGS,
    ACCOUNT_CURRENT,
    ACCOUNT_FD
} AccountType;

// Card type enumeration
typedef enum {
    CARD_DEBIT,
    CARD_CREDIT
} CardType;

// Customer profile structure
typedef struct {
    char customerId[11];          // C10001, etc.
    char name[100];
    char dob[11];                 // YYYY-MM-DD format
    char address[200];
    char email[100];
    char mobileNumber[15];
    KYCStatus kycStatus;
    CustomerStatus status;
    time_t createdAt;
    time_t lastLogin;
} CustomerProfile;

// Account structure
typedef struct {
    char accountId[11];           // A20001, etc.
    char customerId[11];
    AccountType accountType;
    float balance;
    char branchCode[10];
    AccountStatus accountStatus;
    time_t createdAt;
    time_t lastTransaction;
} Account;

// Card structure
typedef struct {
    char cardId[11];              // D30001, etc.
    char accountId[11];
    int cardNumber;
    CardType cardType;
    char expiryDate[11];          // YYYY-MM-DD format
    CardStatus status;
    char pinHash[65];             // SHA-256 hash is 64 hex characters + null terminator
} Card;

// Transaction structure
typedef struct {
    char transactionId[11];       // T40001, etc.
    char accountId[11];
    char transactionType[20];     // "Deposit", "Withdrawal", "Transfer", "Fee"
    float amount;
    time_t transactionTime;
    bool transactionStatus;       // true for success, false for failure
    char transactionRemarks[200];
} Transaction;

// Virtual wallet structure
typedef struct {
    char walletId[11];            // W50001, etc.
    char userId[11];
    float balance;
    time_t lastRefillTime;
    float refillAmount;
} VirtualWallet;

// Load customer profile by card number
bool loadCustomerProfileByCardNumber(int cardNumber, CustomerProfile *profile);

// Load account by card number
bool loadAccountByCardNumber(int cardNumber, Account *account);

// Load card by card number
bool loadCardByCardNumber(int cardNumber, Card *card);

// Get recent transactions for an account
int getRecentTransactions(const char *accountId, Transaction *transactions, int maxTransactions);

// Load virtual wallet for a user
bool loadVirtualWallet(const char *userId, VirtualWallet *wallet);

// Save updated customer profile
bool saveCustomerProfile(const CustomerProfile *profile);

// Save updated account
bool saveAccount(const Account *account);

// Save updated card
bool saveCard(const Card *card);

// Record a new transaction
bool recordTransaction(const Transaction *transaction);

// Update virtual wallet balance
bool updateVirtualWallet(const VirtualWallet *wallet);

#endif // CUSTOMER_PROFILE_H