#ifndef DATABASE_H
#define DATABASE_H

// Card and account operations
float fetchBalance(int cardNumber);
int updateBalance(int cardNumber, float newBalance);

// Card validation operations
int validateCard(int cardNumber, int pin);
int isCardActive(int cardNumber);
int getCardHolderName(int cardNumber, char *name, int maxLen);

// Withdrawal limit management
float getDailyWithdrawals(int cardNumber);
void logWithdrawal(int cardNumber, float amount);
void resetDailyWithdrawals();

// Account management operations
int updatePIN(int cardNumber, int newPIN);
int blockCard(int cardNumber);
int unblockCard(int cardNumber);

#endif // DATABASE_H