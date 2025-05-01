#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "../../../include/atm/transaction/transaction_types.h"

// Database initialization and cleanup
bool initialize_database(void);
void close_database(void);

// Account creation and management
bool createNewAccount(const char* name, const char* address, const char* phone, 
                     const char* email, const char* accountType, float initialDeposit,
                     int* newAccountNumber, int* newCardNumber);
                     
bool generateAccountReceipt(const char* name, long accountNumber, const char* accountType,
                           const char* ifscCode, const char* cardNumber, 
                           const char* expiryDate, int cvv);

// Card and account validation functions
bool doesCardExist(int cardNumber);
bool isCardActive(int cardNumber);
bool validateCard(int cardNumber, int pin);
bool validateCardWithHash(int cardNumber, const char* pinHash);
bool validateRecipientAccount(int recipientCard);
bool validateCardCVV(int cardNumber, int cvv);
bool validateCardExpiry(int cardNumber, const char* expiryDate);
bool validateFullCardDetails(const char* cardNumberStr, const char* expiryDate, const char* cvv, const char* pinHash);

// PIN management functions
bool updatePIN(int cardNumber, int newPin);
bool updatePINHash(int cardNumber, const char* pinHash);

// Card holder information functions
bool getCardHolderName(int cardNumber, char* name, size_t nameSize);
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);

// Account balance functions
bool fetchBalance(int cardNumber, float* balance);
bool updateBalance(int cardNumber, float newBalance);

// Withdrawal tracking functions
void logWithdrawal(int cardNumber, float amount);
void logWithdrawalForLimit(int cardNumber, float amount, const char* date);
float getDailyWithdrawals(int cardNumber);

// Card status management functions
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);
bool updateCardStatus(int cardNumber, bool isActive);

// Transaction logging function
bool logTransaction(int cardNumber, const char* transactionType, float amount, bool success);

// System management
bool getServiceStatus(bool* isActive);
bool setServiceStatus(bool isActive);

// Utility functions
char* generateCardNumber();
int generateCVV();
bool isAccountNumberUnique(int accountNumber);
bool isCardNumberUnique(int cardNumber);

#endif // DATABASE_H