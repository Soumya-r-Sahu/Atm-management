/**
 * @file admin_menu.h
 * @brief Admin menu for the ATM Management System CLI
 */

#ifndef ADMIN_MENU_H
#define ADMIN_MENU_H

/**
 * @brief Run the admin menu
 * @param username Admin username
 * @param userType User type (2 = Admin, 4 = Super Admin)
 */
void runAdminMenu(const char *username, int userType);

#endif /* ADMIN_MENU_H */
