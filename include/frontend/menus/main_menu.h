/**
 * @file main_menu.h
 * @brief Header file for main menu in the Core Banking System
 */

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

/**
 * @brief Run the main menu of the Core Banking System
 */
void runMainMenu(void);

/**
 * @brief Handle login for different user types
 * @param userType Type of user (0=Admin, 1=Customer, 2=ATM Operator)
 */
void handleLogin(int userType);

/**
 * @brief About the Core Banking System
 * Displays information about the system, version, and developers
 */
void showAboutSystem(void);

/**
 * @brief Display help information
 */
void showHelp(void);

#endif /* MAIN_MENU_H */
