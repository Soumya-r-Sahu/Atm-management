/**
 * @file transaction_operations_menu.h
 * @brief Header file for transaction operations menu in the Core Banking System
 */

#ifndef TRANSACTION_OPERATIONS_MENU_H
#define TRANSACTION_OPERATIONS_MENU_H

/**
 * @brief Run the transaction operations menu
 * @param username Customer username
 */
void runTransactionOperationsMenu(const char *username);

/**
 * @brief Perform a deposit transaction
 * @param username Customer username
 */
void performDeposit(const char *username);

/**
 * @brief Perform a withdrawal transaction
 * @param username Customer username
 */
void performWithdrawal(const char *username);

/**
 * @brief Perform a funds transfer
 * @param username Customer username
 */
void performFundsTransfer(const char *username);

#endif /* TRANSACTION_OPERATIONS_MENU_H */
