#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include <stddef.h>
#include "../transaction/transaction_types.h"  // Include shared TransactionType definition

// Card and account validation functions
bool doesCardExist(int cardNumber);
bool isCardActive(int cardNumber);
bool validateCard(int cardNumber, int pin);
bool validateCardWithHash(int cardNumber, const char* pinHash);

// PIN management functions
bool updatePIN(int cardNumber, int newPin);
bool updatePINHash(int cardNumber, const char* pinHash);

// Card holder information functions
bool getCardHolderName(int cardNumber, char* name, size_t nameSize);
bool getCardHolderPhone(int cardNumber, char* phone, size_t phoneSize);

// Account balance functions
float fetchBalance(int cardNumber);
bool updateBalance(int cardNumber, float newBalance);

// Withdrawal tracking functions
void logWithdrawal(int cardNumber, float amount);
void logWithdrawalForLimit(int cardNumber, float amount, const char* date);
float getDailyWithdrawals(int cardNumber);

// Card status management functions
bool blockCard(int cardNumber);
bool unblockCard(int cardNumber);

// Transaction logging function
void logTransaction(int cardNumber, TransactionType type, float amount, bool success);

#endif // DATABASE_H