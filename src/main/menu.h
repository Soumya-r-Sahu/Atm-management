#ifndef MENU_H
#define MENU_H

// Main menu function
void displayMainMenu(int cardNumber);

// Handler functions for menu options
void handleBalanceCheck(int cardNumber, const char *username, const char *phoneNumber);
void handleDeposit(int cardNumber, const char *username, const char *phoneNumber);
void handleWithdrawal(int cardNumber, const char *username, const char *phoneNumber);
void handleMoneyTransfer(int cardNumber, const char *username, const char *phoneNumber);
void handleMiniStatement(int cardNumber, const char *username, const char *phoneNumber);
void handlePinChange(int cardNumber, const char *username);

#endif // MENU_H