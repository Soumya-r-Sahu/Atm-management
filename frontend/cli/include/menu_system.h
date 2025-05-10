/**
 * @file menu_system.h
 * @brief Menu system organization for the ATM Management System CLI
 */

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <stdbool.h>

// User types
typedef enum {
    USER_CUSTOMER_REGULAR = 1,
    USER_ADMIN = 2,
    USER_CUSTOMER_PREMIUM = 3,
    USER_ADMIN_SUPER = 4
} UserType;

// Menu types
typedef enum {
    MENU_MAIN,
    MENU_CUSTOMER,
    MENU_ADMIN,
    MENU_ATM_OPERATIONS,
    MENU_TRANSACTION,
    MENU_CBS_ADMIN
} MenuType;

// Function prototypes for main menu
void displayMainMenu(void);
void handleMainMenuChoice(int choice);

// Function prototypes for customer menu
void runCustomerMenu(const char *username, int userType);

// Function prototypes for customer account submenu
void runAccountMenu(const char *username);
void viewAccountDetails(const char *username);
void changePin(const char *username);
void viewTransactionHistory(const char *username);

// Function prototypes for customer transaction submenu
void runTransactionMenu(const char *username);
void transferFunds(const char *username);
void payBill(const char *username);

// Function prototypes for customer beneficiary submenu
void runBeneficiaryMenu(const char *username);
void viewBeneficiaries(const char *username);
void addBeneficiary(const char *username);

// Function prototypes for ATM operations menu
void runATMOperationsMenu(int cardNumber);

// Function prototypes for admin menu
void runAdminMenu(const char *username, int userType);
void runUserManagementMenu(const char *username, int userType);
void runSystemConfigMenu(const char *username, int userType);
void runReportsMenu(const char *username, int userType);

// Function prototypes for CBS admin menu
void runCBSAdminMenu(void);

#endif /* MENU_SYSTEM_H */
