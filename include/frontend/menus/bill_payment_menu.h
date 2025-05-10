/**
 * @file bill_payment_menu.h
 * @brief Header file for bill payment menu in the Core Banking System
 */

#ifndef BILL_PAYMENT_MENU_H
#define BILL_PAYMENT_MENU_H

/**
 * @brief Run the bill payment menu
 * @param username Customer username
 */
void runBillPaymentMenu(const char *username);

/**
 * @brief Pay a bill for a customer
 * @param username Customer username
 */
void payBill(const char *username);

/**
 * @brief View bill payment history
 * @param username Customer username
 */
void viewBillPaymentHistory(const char *username);

#endif /* BILL_PAYMENT_MENU_H */
