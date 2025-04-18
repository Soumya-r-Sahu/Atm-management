#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void checkBalance(float balance);
float depositMoney(float balance);
float withdrawMoney(float balance);
void changePIN(int *pin);
void exitATM();
void logTransaction(const char *transaction);
void readTransactions();
void savePIN(int cardNumber, int pin);

#endif // DATABASE_H