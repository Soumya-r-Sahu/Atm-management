/**
 * @file customer_menu.h
 * @brief Customer menu for the ATM Management System CLI
 */

#ifndef CUSTOMER_MENU_H
#define CUSTOMER_MENU_H

/**
 * @brief Run the customer menu
 * @param username Customer username
 * @param userType User type (1 = Regular Customer, 3 = Premium Customer)
 */
void runCustomerMenu(const char *username, int userType);

#endif /* CUSTOMER_MENU_H */
