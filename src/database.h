#ifndef DATABASE_H
#define DATABASE_H

void checkBalance(float balance);
float depositMoney(int cardNumber, const char *username);
void withdrawMoney(float *balance, int cardNumber);
void changePIN(int *pin); // Add this declaration
void exitATM(int cardNumber);
int fetchUsername(int cardNumber, char *username);
void savePIN(int cardNumber, int pin);
float fetchBalance(int cardNumber);
float fetchBalanceFromFile(int cardNumber);
void updateBalance(int cardNumber, float newBalance);
void writeTransactionLog(const char *accountHolderName, const char *operation, const char *details);
void readTransactionLogs();
int fetchServiceStatus(); // Function to fetch the ATM service status from status.txt
float getDailyWithdrawals(int cardNumber);
void logWithdrawal(int cardNumber, float amount);
void viewTransactionHistory(int cardNumber);

#endif // DATABASE_H