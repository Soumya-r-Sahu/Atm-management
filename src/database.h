#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations
void checkBalance(float balance);
float depositMoney(int cardNumber, const char *username);
float withdrawMoney(float balance);
void changePIN(int *pin);
void exitATM(int cardNumber);
int fetchUsername(int cardNumber, char *username);
void savePIN(int cardNumber, int pin);
float fetchBalance(int cardNumber); // Add this declaration
void updateBalance(int cardNumber, float newBalance); // Add this declaration
void writeTransactionLog(const char *username, const char *operation, const char *details); // Add this declaration
void readTransactionLogs();

#endif // DATABASE_H