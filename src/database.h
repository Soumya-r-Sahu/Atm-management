#ifndef DATABASE_H
#define DATABASE_H

// ============================
// Balance and Transaction Functions
// ============================
void checkBalance(float balance);
float fetchBalance(int cardNumber);
float fetchBalanceFromFile(int cardNumber);
void updateBalance(int cardNumber, float newBalance);
float depositMoney(int cardNumber, const char *username);
void withdrawMoney(float *balance, int cardNumber);
float getDailyWithdrawals(int cardNumber);
void logWithdrawal(int cardNumber, float amount);

// ============================
// PIN and Account Management
// ============================
void changePIN(int *pin);
void savePIN(int cardNumber, int newPIN);
void updateAccountStatus(int cardNumber, const char *status);
int fetchUsername(int cardNumber, char *username);

// ============================
// Logging and Transaction History
// ============================
void writeTransactionLog(const char *accountHolderName, const char *operation, const char *details);
void readTransactionLogs();
void viewTransactionHistory(int cardNumber);

// ============================
// ATM Service and Utility Functions
// ============================
int fetchServiceStatus(); // Function to fetch the ATM service status from status.txt
void exitATM(int cardNumber);

#endif // DATABASE_H