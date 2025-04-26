#ifndef MENU_H
#define MENU_H

// Menu display functions
void displayMenu();
void displayOthersMenu();
void displayCardServicesMenu();
void displayFastCashMenu();

// Menu handlers
void handleUserChoice(int choice, int cardNumber, const char* username);
void handleOthersMenu(int cardNumber, const char* username);
void handleCardServices(int cardNumber, const char* username);
void handleFastCash(int cardNumber, const char* username);
void handleFundTransfer(int cardNumber, const char* username);

// Account type related functions
typedef enum {
    ACCOUNT_SAVINGS,
    ACCOUNT_CURRENT,
    ACCOUNT_CREDIT
} AccountType;

const char* getAccountTypeString(AccountType type);
AccountType selectAccountType();

#endif // MENU_H