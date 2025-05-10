/**
 * @file account_menu.h
 * @brief Header file for account management menu in the Core Banking System
 */

#ifndef ACCOUNT_MENU_H
#define ACCOUNT_MENU_H

/**
 * @brief Run the account management menu
 * @param username Customer username
 */
void runAccountMenu(const char *username);

/**
 * @brief View account details for a customer
 * @param username Customer username
 */
void viewAccountDetails(const char *username);

/**
 * @brief View transaction history for a customer
 * @param username Customer username
 */
void viewTransactionHistory(const char *username);

/**
 * @brief Change PIN for a customer account
 * @param username Customer username
 */
void changePin(const char *username);

#endif /* ACCOUNT_MENU_H */
